'use strict';

var raw = require ("raw-socket");

//
//  ICMP ECHO header
//
let header = [];

let type            = new Buffer(1);
let code            = new Buffer(1);
let identifier      = new Buffer(2);
let sequence_number = new Buffer(2);
let chksum          = new Buffer(2);
let data            = new Buffer(4);

type.writeUInt8(0x8, 0);
code.writeUInt8(0x0, 0);
chksum.writeUInt16LE(0x0, 0);
identifier.writeUInt16LE(0x4, 0);
sequence_number.writeUInt16LE(0x0, 0);
data.writeUInt32LE(0x0, 0);

header.push(type);
header.push(code);
header.push(chksum);
header.push(identifier);
header.push(sequence_number);
header.push(data);

let buf = new Buffer(2);
let int = checksum(header);
buf.writeUInt16LE(int);

header[2] = buf;

let headerConcat = new Buffer.concat(header);

//
//  Creatign the socket uign the ICMP protocol
//
var socket = raw.createSocket({protocol: raw.Protocol.ICMP});


//  
//  Sendign the request for a ping
//  
socket.send(headerConcat, 0, 12, "8.8.8.8", function(error, bytes) {

    // 
    //  If there is any error, show it.
    // 
    if (error) {
        console.log(error.toString());
    }

});

socket.on("message", function (buffer, source) {

    // 
    //  Create a buffer that will hold just our ICMP reaply, we don't need the
    //  whole TCP blob :)
    // 
    let icmpResponseBuffer = new Buffer(8);
    
    // 
    //  Copy only the fragment from the response that interest us,
    //  startign at byte 20
    // 
    buffer.copy(icmpResponseBuffer, 0, 20);

    // 
    //  Create all the buffers where we are goignto store the different
    //  information from the ICMP reply.
    // 
    let type = new Buffer(1);
    let code = new Buffer(1);
    let checksum = new Buffer(2);
    let identifier = new Buffer(2);
    let sequence_number = new Buffer(2);
    
    // 
    //  Copy bytes in to the apropierate buffer
    // 
    icmpResponseBuffer.copy(type, 0, 0);
    icmpResponseBuffer.copy(code, 0, 1);
    icmpResponseBuffer.copy(checksum, 0, 2);
    icmpResponseBuffer.copy(identifier, 0, 4);
    icmpResponseBuffer.copy(sequence_number, 0, 6);
    
    //
    //  Display in a human readable form the resposne that we got
    //
    console.log("type: %s, code: %s, checksum: %s, identifier: %s, sequence: %s",
                type.toString('hex'),
                code.toString('hex'),
                checksum.toString('hex'),
                identifier.toString('hex'),
                sequence_number.toString('hex')
            );

    //
    //  Once we have our response we can exit the app
    //
    process.exit()

});

function checksum(array) {

    let buffer = new Buffer.concat(array);
    let nleft = 12;
    let sum = 0;
    let answer = 0;
    let w = 0;
    
    for(let i = 0; i < 2; i++)
    {
        w += parseInt(buffer[i]);
    }
    
    let position = 0;

    while(nleft > 1)
    {
        sum += parseInt(buffer[position++]);
        nleft -= 2;
    }

    if(nleft == 1)
    {
        sum += 8;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    
    let uintResult = (new Uint16Array([answer]))[0];

    return uintResult;

}
