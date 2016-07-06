'use strict';

let raw = require ("raw-socket");
let header = [];

//
//	1. Create the structure where we are going to
//	   save our header
//
let type            = new Buffer(1);
let code            = new Buffer(1);
let identifier      = new Buffer(2);
let sequence_number = new Buffer(2);
let chksum          = new Buffer(2);
let data            = new Buffer(4);

//
//	2. Write the appropriate values.
//
type.writeUInt8(0x8, 0);
code.writeUInt8(0x0, 0);
identifier.writeUInt16LE(0x0, 0);
sequence_number.writeUInt16LE(0x0, 0);
chksum.writeUInt16LE(0xfff7, 0);
data.writeUInt32LE(0x0, 0);

//
//	3. push each separated buffer to the array
//
header.push(type);
header.push(code);
header.push(chksum);
header.push(identifier);
header.push(sequence_number);
header.push(data);

//
//	4. Combine all the buffers in to one
//
let headerConcat = new Buffer.concat(header, 12);

//
//	5. Creating the socket using the ICMP protocol
//
var socket = raw.createSocket(
	{
		protocol: raw.Protocol.ICMP
	}
);

//
//	6. Sending the request for a ping
//
socket.send(headerConcat, 0, 12, "8.8.8.8", function(error, bytes)
	{
	    //
	    //	-> If there is any error, show it.
	    //
	    if (error)
	    {
	        console.log(error.toString());
	    }
	}
);

//
//	7. Listen for the remote host response
//
socket.on("message", function (buffer, source)
	{
		//
		//	-> Show the response message
		//
	    console.log ("Received " + buffer.length + " bytes from " + source);

		//
    	//  8. Once we have our response we can exit the app
    	//
	    process.exit()
	}
);
