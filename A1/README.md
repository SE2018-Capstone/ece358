# Assignment 1

`make debug` - Runs make with `DEBUG` defined.  The `INFO` calls will not print anything if `DEBUG` isn't defined

`structs.h` - Defines all the message formats that get sent between peers


### Pseudocode plan:

Each peer keeps track of p (num peers) and c (num content)

- addpeer
   - added peer code:
```
insert self between given peer and its next peer
get c from next peer
numRequested = floor(c/p+1)
numAboveMin = c%p
numMin = p - numAboveMin
aboveMax = (ceil(c/p)-ceil(c/(p+1)))*numAboveMin
minimumsAboveMax = (floor(c/p)-ceil(c/(p+1)))*numMin
if minimumsAboveMax > 0:
  aboveMax += minimumsAboveMax
numRequested -= aboveMax
p = p+1
send numRequested and new p to next peer
wait for numRequested content responses
```
   - peer code:
```
if message is p, numRequested:
  set p to new val
  while numItemsIHold > ceil(c/p):
    remove any content
    send content package to next peer
  if numRequested > 0 and numItemsIHold > floor(c/p):
    remove any content
    send content package to next peer
  send p, numRequested to next peer
else if message is content:
  forward content to next peer

```

- removepeer
   - removed peer code:
```
numExtra = numItemsIHold
numAboveMin = c%p
numMin = p - numAboveMin
numBelowMin = (floor(c/p-1)-floor(c/(p)))*numMin
maxiumumsBelowMin = (floor(c/p-1)-ceil(c/(p)))*numAboveMin
if maximumsBelowMin > 0:
  numBelowMin += maximumsBelowMin
numExtra -= numBelowMin
p -= 1
send new p and numExtra to next peer
while numItemsIHold > 0:
  send content package to next peer
remove self from ring
```
   - peer code:
```
if message is p, numExtra:
  update p
  if numItemsIHold < ceil(c/p):
    numExtra--;
    willTakeExtraContent = true
  forward p, numExtra to next peer
else if message is content:
  if numItemsIHold < floor(c/p):
    save content
  else if willTakeExtraContent:
    save content
  else:
    forward content to next peer
```

- addcontent
   - peer code:
```
set c to new val
if content exists:
  if numItemsIHold < ceil(c/p):
    save content
    pass c to next peer
  else:
    pass c and content to next peer
else:
  pass c to next peer
```
   - first peer code:
```
establish connection with console process
c = c+1
key = c
content = (key, string)
execute normal peer code
send key to console process
```

- removecontent
   - peer code:
```
if key is mine and numItemsIHold < floor(c/p):
  remove items[key]
  IRequestedContent = true
  c = c-1
  send updated c and request content to next peer
else if key is mine:
  remove items[key]
  c = c-1
  send updated c to next peer
else if key is present:
  send key to next peer
else if request content is present:
  update c with new val
  if c%p == 0 and numItemsIHold > c/p:
    select any content from items
    remove it
    send c and content to next peer
  else if c%p != 0 and numItemsIHold == ceil(c/p):
    select any content from items
    remove it
    send c and content to next peer
  else:
    send c and request content to next peer
else if content is present:
  update c with new val
  if IRequestedContent:
    add content
    IRequestedContent = false
  else:
    send content to next peer
```
   - first peer code:
```
establish connection with console process
execute normal peer code
wait until this peer is called:
  if key still being passed around:
    send "no such content" error to console process
  else:
    terminate console connection // it's going to succeed
    execute normal peer code
```

- lookupcontent
   - first peer must remember the address of the requesting console
   - peer code:
```
if (I have content matching key):
  send content to next peer
else:
  pass request to next peer
```
   - first peer code:
```
establish connection with console process
execute normal peer code
wait for content to arrive from peer
send content to console process
```
  
- allkeys
   - return list from internal hashtable
