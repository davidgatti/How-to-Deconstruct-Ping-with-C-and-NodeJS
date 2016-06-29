'use strict';

let raw = require ("raw-socket");
let header = [];

//
//  1. Create the structure where we are going to 
//     save our header
//
let type            = new Buffer(1);
let code            = new Buffer(1);
let chksum          = new Buffer(2);
let identifier      = new Buffer(2);
let sequence_number = new Buffer(2);
let data            = new Buffer(4);

//
//  2. Wrtie the apropriate values.
//
type.writeUInt8(0x8, 0);
code.writeUInt8(0x0, 0);
chksum.writeUInt16LE(0x0, 0);
identifier.writeUInt16LE(process.pid, 0);
sequence_number.writeUInt16LE(0x0, 0);
data.writeUInt32LE(0x0, 0);

//
//  3. push each separated buffer to the array
//
header.push(type);
header.push(code);
header.push(chksum);
header.push(identifier);
header.push(sequence_number);
header.push(data);

let headerConcat2 = new Buffer.concat(header);

let buf = new Buffer(2);
let int = checksum(header);
buf.writeUInt16LE(int);

header[2] = buf;

//
//  4. Combine all the buffers in to one
//
let headerConcat = new Buffer.concat(header);

//
//  5. Creatign the socket uign the ICMP protocol
//
var socket = raw.createSocket(
    {
        protocol: raw.Protocol.ICMP
    }
);

//
//  6. Sendign the request for a ping
//
socket.send(headerConcat, 0, 12, "8.8.8.8", function(error, bytes)
    {
        // 
        //  -> If there is any error, show it.
        // 
        if (error) 
        {
            console.log(error.toString());
        }
    }
);

//
//  7. Listent for the remote host response
//
socket.on("message", function (buffer, source) {


    // 
    //  8. Create a buffer that will hold just our ICMP reaply, we don't need 
    //     the whole TCP blob :)
    // 
    let icmpResponseBuffer = new Buffer(8);
    
    // 
    //  9.Copy only the fragment from the response that interest us,
    //    startign at byte 20
    // 
    buffer.copy(icmpResponseBuffer, 0, 20);

    // 
    //  10. Create all the buffers where we are goignto store the different
    //      information from the ICMP reply.
    // 
    let type = new Buffer(1);
    let code = new Buffer(1);
    let checksum = new Buffer(2);
    let identifier = new Buffer(2);
    let sequence_number = new Buffer(2);
    
    // 
    //  11. Copy bytes in to the apropierate buffer
    // 
    icmpResponseBuffer.copy(type, 0, 0);
    icmpResponseBuffer.copy(code, 0, 1);
    icmpResponseBuffer.copy(checksum, 0, 2);
    icmpResponseBuffer.copy(identifier, 0, 4);
    icmpResponseBuffer.copy(sequence_number, 0, 6);
    
    //
    //  -> Display in a human readable form the resposne that we got
    //
    console.log("type: %s, code: %s, checksum: %s, identifier: %s, sequence: %s",
                type.toString('hex'),
                code.toString('hex'),
                checksum.toString('hex'),
                identifier.toString('hex'),
                sequence_number.toString('hex')
            );

    //
    //  12. Once we have our response we can exit the app
    //
    process.exit()

});

//
//  () Checksum Method
//
function checksum(array)
{
    //
    //  1. Variable needed for the calculation
    //
    let buffer = new Buffer.concat(array);
    let twoByteContainer = new Buffer(2);
    let position = 0;
    let answer = 0;
    let sum = 0;
    
    //
    //  2. Read only the first two bytes from the buffor 
    //
    for(let i = 0; i < 6; i++)
    {
        //
        //  1. Copy two bytes from the original buffer
        //
        buffer.copy(twoByteContainer, 0, position);

        //
        //  2. read the buffer as unsigned integer in little endian mode
        //
        let decimal = twoByteContainer.readUIntLE(0, 2);

        //
        //  3. Increse the position by two
        //
        position += 2
    
        //
        //  4. Sum the new value with the previous one
        //
        sum += decimal;
    }

    //
    //  3. Needed conversions
    //
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    
    //
    //  4. Invert the bits
    //    
    answer = ~sum;
    
    //
    //  5. Interprete the number as Unsigned
    //    
    let uintResult = (new Uint16Array([answer]))[0];

    //
    //  -> Result
    //
    return uintResult;
}
