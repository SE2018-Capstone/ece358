# Assignment 1

`make debug` - Runs make with `DEBUG` defined.  The `INFO` calls will not print anything if `DEBUG` isn't defined

`structs.h` - Defines all the message formats that get sent between peers


### Pseudocode plan:

Each peer keeps track of p and c

- addpeer

- removepeer

- addcontent
   - Go around the network, notify new c and new content
   - If you’re under the ceiling, then accept it, but continue to pass new c
   - otherwise pass new c and content along

- removecontent
   - send around to the person that needs it
   - If removing him doesn’t put him below floor

- lookupcontent
   - Send message to other dude with request
   - If receive request, check if you’ve got it, if you have it, send repsonse
     around network
   - If receive response for key that you’re looking up, accept it and don’t
     forward

- allkeys
   - return list from internal hashtable
