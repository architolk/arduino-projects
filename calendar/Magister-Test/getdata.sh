#!/bin/bash
apikey=$(grep '#define FEED_1' ../Calendar/src/APIClients/secrets.h | sed -n 's/.*"\(.*\)".*/\1/p')
curl https://calendar.magister.net/api/icalendar/feeds/$apikey > feed1.txt
