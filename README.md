# Deconstructing Ping with C and NodeJS

I’m David Gatti, and my goal with this repository is to demystify again the word protocols and more specific this time: structs in C. This also is another article in a series of articles where I try to learn myself something new, and prove that there is nothing to hard to learn, we just need to pass through the unknown zone. On the other side the weather is clear and sunny.

Before you start, I recommend you read the previous article where I explain in detail all about sockets, using Particle and NodeJS. In this one, we are going to use sockets but this time, we are going to focus on how to work with a binary protocol, and are going to craft our own ICMP header, and read the response from the remote host.

I know that the word protocol, binary, and crafting - might seams hard or complicated, but I can guarantee you that it is not the case. 

# How to understand a header specification

If you visit the wikipedia page that talk about the ICMP protocol, you’ll find this table that describes the header that needs to be sent over the wire to actually make a Ping request.

lets start by understanding that we are talking about a binary protocol, meaning we are going to send bytes, which are number. Those numbers are as is, they are not a ASCII representation of the keyboard character set for example. 

One byte is 8 bits, which means a integer of value 8 is 00001000, and this is not a ASCII 8, the same letter that you are reading right now, meaning we can’t type just the number 8 in our keyboard.

To make our life easier, we are going to write our data in Hex (short for Hexadecimal). This format is way more manageable, because instead of writing numbers as integers, we are going to write an integer in a set of 2 characters 38450 becomes 0x9632, and we can display this 96 32, putting a space very two numbers because two number in Hex are one byte, making it easier to debug in the console. 

# Lets brake down the table above

It starts from 00 to 31, which means each row consists of 32bits, which if we divide by 8 bits, gives us 4 bytes. The table has 3 rows, so in total we are going to send 12 bytes.

The first row consists of 3 data sets, 1 byte for the type (uint8_t), 1 byte for the code (uint8_t), and two bytes for the check sum (uint16_t). This could look like this: 08 00 FF F7.

The second row has 2 bytes (uint16_t) for the identifier, and two for the sequence number, for example: 09 A0 00 01

The third row is the payload which is optional, you could send some random data, and the router would have to return that date back to you, useful if you want to check if data is not being lost down the way. But in our example, we are not going to use this.

# Why NodeJS in this project

This project has NodeJS to show the difference between a low level language and a high level one. NodeJS can handle sockets very well, but there are different types of them which lives in different parts of the OSI model https://en.wikipedia.org/wiki/OSI_model. As you can see from the Wikipedia table, TCP and UDP lives on the 4th layer of the model, which NodeJS can handle. But from the Examples column you see that ICMP lives on the 3th layer, and hear is where NodeJS can’t reach anymore. We will be able to still ping from NodeJS, but I’ll explain how later.

# The file structure

As you can see from the repository, there are two folders: C and NodeJS. Each folder have 3 files that are named the same to help you match each example from one language to the other. And so:

- **pingWithBareMinimum**: this file will create a PING with the bare minimum code needed to do so, so we can focus in understanding how to build our own header, and get a result from the remote machine.
- **pingWithStructResponse**: here is where we are going to apply our knowledge from before but this time, we are going to store the result in a C struct, and a buffers in the case of NodeJS
- **pingWithChecksum**: here is where we implement the whole thing, so we can actually send a proper ping with changing data.

# Lets start with C

Section 3 in the pingWithBareMinimum.c file is the part that interest us the most. Here is where we actually create the ICMP header. First of all we are describing a struct which is our header, the same header that I described above, while explaining the table from Wikipedia. To make sure we are on the same page:

- uint8_t: means 8 bits, which is 1 byte
- uint16_t: means 16 bits, which is 2 bytes
- uint32_t: means 32 bits, which is 4 bytes 

Once we have our struct done, we basically just created our own data type. That is why we are typing icmp_hdr_t pckt;, where icmp_hdr_t  is the type that we created and are creating a variable of this type. 

The following code just uses the newly created struct, and adds the required data. One important thing notice, is the data field. We don’t need to write 4 zeros to fill the 32bit space, because when we create the variable with our struct, the memory is already assigned, since we explicitly say: uint32_t data;, and the compiler knows that we are using 4 bytes of memory.

The next part is the IP header which as you can see also uses a struct, but this time we are not making it, because someone else already did it in one of the libraries that we import. We could make our own, we would just need to follow the IP protocol. But we are lazy, so lets use what other did.

Once we have all this, we can use our socket that we created at the beginning of the file, and send our header, and if we did everything correctly, in the next section 4 we are going to get a nice replay from the remote host.

# Read the replay

Until now we created the most basic ping possible. In this section we are going to focus on the response, and more specific how to get the response and map it to a struct so we can have easy access to the data within our code. Lets open the pingWithStructResponse.c file and get down.

Scroll down to section 4, and you’ll see that there is a new struct in place. In this place we are again telling the compiler how much space are we going to need and how it should be divided. 

On line 110 you can see that we are creating a variable using our struct, and in the line bellow we are casting the buffer to our variable starting at the 20th byte. Since everything before the 20th is data related to the IP and TCP protocol which we don’t care about, we just want to see what message did we get from the remote host.

After mapping the data to a structure you can see in the next line that we are able to log each peace of information very easily.

# All in

We can make a request, we can easily read the reply , but we can’t yet fully use the whole protocol since it lacs the checksum function that should calculate the check sum. Right now we kept sending a fixed checksum since we don't pass the identifier nor the sequence_number. 

Now it is the right time to keep improving our code, and make it better. 

Regarding the new variables from file pingWithChecksum.c:

- **identifier**: is a number that if we send, it will be sent back to us, the idea is then that we can identify if the ping came from our app, or someone else. Meaning in the replay we could filter the incoming ping responses and only show ours.
- **sequence_number**: is a number that is going also to be sent back to us. It is useful to see if we lost some packets, because the idea is to increment this number whenever we make a ping request.

Adding this values will make each request unique, thus we need to calculate the checksum, otherwise the ping will be stopped within our network by our home router.

The checksum function works by taking the first 6 bytes and summing them all together, and in the end, we revers the bits with a NOT operation.

We are not going to use the data field, but if we did, then there is one last peace of code that would get that data and sum it up to the rest of the checksum. 

# The difference between NodeJS and C

Until now we covered the only the C code in this project, but why even put NodeJS here? Well I like to see what are the limits of this environment for once, and use it as an example to show the difference between a low level language like C, and a higher level language like NodeJS.

Since the file names are exactly the same apart for the format, you can easily open the matching files, and see how different it is to do certain things. For example, it is way easier in C to create a binary header thanks to structs, and also map a buffer to such struct to have easy access to the data.

As mentioned in the beginning of this article, we know that NodeJS works on a higher layer of the OSI model. Because of that, yes we wrote our now header code to make a ping request in NodeJS, but we had to use an external module called raw-socket, which is basically C wrapped in Javascript for easy use in NodeJS.

This also means that this module might work differently under different system since it uses the sockets that the system have available. The author of this module of course tried to make sure his work will work in as many places possible, but different system like Windows and MacOS don’t adhere to the POSIX standard. Highly recommend reading what he wrote on his project READEM.md file.

# Was it that bad?

And you made it :) I hope this article was clear and to the point (of course if you would like me to explain something in more detail fell free to ask me). I hope by now no protocol will scare you anymore, and by now this word should make you exited to learn how a peace of software is able to communicate with another remote device, might it be a computer, IoT, or even radio. 