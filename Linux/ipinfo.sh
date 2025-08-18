#!/bin/bash
pubIP=$(curl -s ifconfig.me)
echo "Public IP is $pubIP"
curl -s "http://ipwhois.app/json/$pubIP" | jq -r '"Country: " + .country, "Region: " + .region, "City: " + .city'
echo \nIPAPI says\n
curl -s https://ipapi.co/json | jq '.country,.region,.city,.ip,.org'
