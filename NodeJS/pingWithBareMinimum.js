'use strict';

var raw = require ("raw-socket");

/**
 *
 * ICMP ECHO header
 *
 */

let header = [];

let type            = new Buffer(1);
let code            = new Buffer(1);
let identifier      = new Buffer(2);
let sequence_number = new Buffer(2);
let chksum          = new Buffer(2);
let data            = new Buffer(4);

type.writeUInt8(0x8, 0);
code.writeUInt8(0x0, 0);
identifier.writeUInt16BE(0x0, 0);
sequence_number.writeUInt16BE(0x0, 0);
chksum.writeUInt16BE(0xf7ff, 0);
data.writeUInt32BE(0x0, 0);

header.push(type);
header.push(code);
header.push(identifier);
header.push(sequence_number);
header.push(chksum);
header.push(data);

let headerConcat = new Buffer.concat(header, 12);

/**
 *
 * Creatign the socket uign the ICMP protocol
 *
 */

var socket = raw.createSocket({protocol: raw.Protocol.ICMP});

/**
 *
 * Sendign the request for a ping
 *
 */
socket.send(headerConcat, 0, 12, "8.8.8.8", function(error, bytes) {

    // If there is any error, show it.
    if (error) {
        console.log(error.toString());
    }

});

socket.on("message", function (buffer, source) {

    console.log ("Received " + buffer.length + " bytes from " + source);

    // Once we have our response we can exit the app
    process.exit()

});
