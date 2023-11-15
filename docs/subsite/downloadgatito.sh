#!/bin/bash
echo -e -n "HTTP/1.1 200 OK\r\n"
echo -e -n "Content-Type: image/jpeg\r\n"
echo -e -n "Content-Disposition: attachment; filename=\"elgatitohehehe.jpg\"\r\n"
echo -e -n "\r\n"

path=$(pwd)
path+="/docs/subsite/gatito.jpg"

cat $path
