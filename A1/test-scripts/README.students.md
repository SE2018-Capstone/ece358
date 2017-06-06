Instructions for Students
=========================

## Step 0: Unpacking the Tests
1. Unzip `ece358-programming-assignment-1-test-scripts.zip` to the root folder where your `Makefile` resides.
1. After unzipping, your directory should look similar to this:
    ~~~~
    [jmshahen@ecelinux2 Downloads]$ ls -1
    Makefile
    README.students.md
    src
    tests
    ~~~~
1. In order to run any of the provided tests you **MUST BE RUNNING ON ecelinux1, ecelinux2, or ecelinux3**
    ~~~~bash
    echo From home SSH into ecelinux4
    ssh ecelinux4
    echo Immediately ssh into ecelinux1, ecelinux2, or ecelinux3
    ssh ecelinux2
    ~~~~
1. **CONFIRM YOU ARE ON ecelinux1, ecelinux2, or ecelinux3**, your prompt will look like this: `[jmshahen@ecelinux2 ~]$ `
1. Are you sure you are on ecelinux1, ecelinux2, or ecelinux3?


# Step 1: Running Test01
**You must run TEST01 before running TEST02!**

~~~~
[jmshahen@ecelinux1 Downloads]$ ls
Makefile  README.students.md  src  tests
[jmshahen@ecelinux1 Downloads]$ ls -1
Makefile
README.students.md
src
tests
[jmshahen@ecelinux1 Downloads]$ python tests/test01.py
Grade: 10 / 10
You passed with full marks! Congratulations :)

Grades Received: +9 for no errors running make
Grades Received: +1 for all files accurately created
~~~~

# Step 2: Running Test02
**Did you pass test01? You can only run test02 when you have passed test01**

~~~~
[jmshahen@ecelinux1 Downloads]$ ls
addcontent  allkeys      lookupcontent  README.students.md  removepeer  tests
addpeer     grading.csv  Makefile       removecontent       src
[jmshahen@ecelinux1 Downloads]$ python tests/test02.py
Grade: 23 / 30
You did not get perfect on this test, please review the following comments:

Grades Deducted: [allkeys] -2% -- Too many keys found in the system: peer1 keys['10']; peer2 keys:['11']
Grades Deducted: [lookupcontent] -1% -- Able to find content that should not exist with key: 11
Grades Deducted: [removecontent/lookupcontent] -2% -- Able to find content that was just removed with key: 11
Grades Deducted: [removecontent/lookupcontent] -2% -- Able to find content that was just removed with key: 11

Grades Received: +1 for addpeer test
Grades Received: +1 for addpeer/2 peers test
Grades Received: +1 for addpeer/2 peers/non-local ip test
Grades Received: +1 for addpeer test
Grades Received: +2 for addcontent test
Grades Received: +1 for addcontent test
Grades Received: +2 for addcontent test
Grades Received: +3 for allkeys test
Grades Received: +3 for lookupcontent test
Grades Received: +1 for lookupcontent test
Grades Received: +2 for removepeer test
Grades Received: +1 for addpeer/2 simultaneous networks test
Grades Received: +3 for removepeer test
~~~~