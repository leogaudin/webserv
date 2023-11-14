#!/bin/bash
echo -e "HTTP/1.1 200 OK"
echo -e "Content-Type: image/jpeg\r\n"

path=$(pwd)
path+="/docs/subsite/gatito.jpg"

cat $path
