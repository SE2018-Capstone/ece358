'''
Test Name : test02
Description : Simple testcase to test each program with the following calls:
01. addpeer
02. addpeer ip2 port2
03. addcontent ip1 port1 'hello'
04. allkeys ip1 port1
05. allkeys ip2 port2
06. addcontent ip2 port2 'hello world'
07. lookupcontent ip1 port1 key_hello
08. lookupcontent ip2 port2 key2
09. removecontent ip1 port1 key2
10. removepeer ip1 port1
11. addpeer
'''

# pylint: disable=W0403
from common_tasks import new_network, add_peer, add_content, all_keys,lookup_content,remove_content,remove_peer,error_messages
from helper import save_grade

test02_total_grade = 30

def test02():
    '''
    Input : 
    Output : (grade, comments, pos_grades)
        grade=0-30 -- grade the student gets
        grade=-1 -- if an error occurred
    '''
    test02_grade = 0
    test02_comments = []
    test02_pos_grades = []

    #############################################################################
    #############################################################################
    # [+1%] Start a new network
    (peer1, comments) = new_network()

    # Check for errors; if errors then kill test 
    if peer1 is None or len(comments) != 0:
        # errors occurred
        test02_comments.extend(comments)
        # Kill test because no network was created
        return (-1, test02_comments)
    else:
        if not '127.0.0.1' in peer1:
            # not a Local ip address returned
            test02_pos_grades.append('+1 for addpeer test')
            test02_grade += 1
        else:
            test02_comments.append('[addpeer] -1% -- Local ip address (127.0.0.1) returned')

    # [+2%] add a new peer (total of 2 peers in the network)
    (peer2, comments) = add_peer(peer1)

    # Check for errors; if errors then kill test 
    if peer2 is None or len(comments) != 0:
        # errors occurred
        test02_comments.extend(comments)
        # Kill test because no network was created
        return (-1, test02_comments)
    else:
        test02_grade += 1
        test02_pos_grades.append('+1 for addpeer/2 peers test')
        if not '127.0.0.1' in peer2:
            # No errors [+2%]
            test02_pos_grades.append('+1 for addpeer/2 peers/non-local ip test')
            test02_grade += 2
        else:
            test02_comments.append('[addpeer/2 peers] -1% -- Local ip address (127.0.0.1) returned')

    # [+1%] Check if peer1 is different from peer2
    if peer1 != peer2:
        test02_pos_grades.append('+1 for addpeer test')
        test02_grade += 1
    else:
        test02_comments.append('[addpeer] -1% -- returned peers are not unique: peer1({}) == peer2({})'.format(peer1,peer2))
    #############################################################################

    #############################################################################
    #############################################################################
    # add content: 'hello'
    (key_hello, comments) = add_content(peer1, 'hello')

    ## Error Checking: addcontent
    if len(comments) != 0:
        test02_comments.extend(comments)
    else:
        # [+2%] if addcontent returns a key
        test02_pos_grades.append('+2 for addcontent test')
        test02_grade += 2

    # all keys peer1
    (peer1_keys, c1) = all_keys(peer1)
    # all keys peer2
    (peer2_keys, c2) = all_keys(peer2)

    ## Error Checking: allkeys
    if len(c1) != 0 or len(c2) != 0:
        # FATAL ERROR!
        # error occurred with allkeys!
        test02_comments.extend(c1)
        test02_comments.extend(c2)
        return (test02_grade, test02_comments)

    # check if key_hello exists on peer1 or peer2
    if len(peer1_keys) == 1 and len(peer2_keys) == 0:
        if peer1_keys[0] != key_hello:
            test02_comments.append('[allkeys] -2% -- Incorrect key found on {}: {} should have been {}'.format(peer1, peer1_keys[0], key_hello))
        else:
            # [+2%] allkeys returns only 1 key on a single host and none on the other
            test02_pos_grades.append('+2 for addcontent test')
            test02_grade += 2
    elif len(peer2_keys) == 1 and len(peer1_keys) == 0:
        if peer2_keys[0] != key_hello:
            test02_comments.append('[allkeys] -2% -- Incorrect key found on {}: {} should have been {}'.format(peer2, peer2_keys[0], key_hello))
        else:
            # [+2%] allkeys returns only 1 key on a single host and none on the other
            test02_pos_grades.append('+2 for addcontent test')
            test02_grade += 2
    elif len(peer2_keys) == 0 and len(peer1_keys) == 0:
        test02_comments.append('[allkeys] -2% -- No keys found in the network! Should have found key: {}'.format(key_hello))
    else:
        test02_comments.append('[allkeys] -2% -- Too many keys found in the system: peer1 keys{}; peer2 keys:{}'.format(peer1_keys, peer2_keys))

    # add content: 'hello world'
    (key_hello_world, comments) = add_content(peer2, 'hello world')

    ## Error Checking: addcontent
    if len(comments) != 0:
        print 'AA'
        test02_comments.extend(comments)
    else:
        # [+1%] if content with spaces does not work
        test02_pos_grades.append('+1 for addcontent test')
        test02_grade += 1

    # Check if hashes are different
    if key_hello != key_hello_world:
        # [+2%] if 2 content hashes to same value
        test02_pos_grades.append('+2 for addcontent test')
        test02_grade += 2
    else:
        test02_comments.append('[addcontent] -2% -- The keys returned from addcontent should be unique, received keys: \'{}\' and \'{}\''.format(key_hello, key_hello_world))

    # all keys peer1
    (peer1_keys, c1) = all_keys(peer1)
    # all keys peer2
    (peer2_keys, c2) = all_keys(peer2)

    ## Error Checking: allkeys
    if len(c1) != 0 or len(c2) != 0:
        # FATAL ERROR!
        # error occurred with allkeys!
        test02_comments.extend(c1)
        test02_comments.extend(c2)
        return (test02_grade, test02_comments)

    # check if key_hello and key_hello_world exists on peer1 and peer2
    combined_keys = peer1_keys + peer2_keys

    if len(combined_keys) == 2:
        # accurate amount of keys
        passed_test = []
        if not key_hello in combined_keys:
            passed_test.append(key_hello)
        if not key_hello_world in combined_keys:
            passed_test.append(key_hello_world)
        if len(passed_test) == 0:
            # [+3%] allkeys returns the 2 correct keys in the network (does not need to be load balanced)
            test02_grade += 3
            test02_pos_grades.append('+3 for allkeys test')
        else:
            test02_comments.append('[allkeys] -3% -- Unable to find key(s) [{}] on the network: peer1 keys{}; peer2 keys:{}'.format(passed_test, peer1_keys, peer2_keys))
    else:
        test02_comments.append('[allkeys] -3% -- Too many keys found in the system: peer1 keys{}; peer2 keys:{}'.format(peer1_keys, peer2_keys))
    #############################################################################

    #############################################################################
    #############################################################################
    # [+3%] Lookup content: 'hello'
    (content, error,comments) = lookup_content(peer1, key_hello)

    if error == '' and content == 'hello':
        test02_grade += 3
        test02_pos_grades.append('+3 for lookupcontent test')
    elif error == error_messages['no content']:
        test02_comments.append('[lookupcontent] -3% -- Unable to find content "hello" with key: {}'.format(key_hello))
    test02_comments.extend(comments)

    # [+1%] Lookup content: 'hello world'
    (content, error,comments) = lookup_content(peer2, key_hello_world)

    if error == '' and content == 'hello world':
        test02_grade += 1
        test02_pos_grades.append('+1 for lookupcontent test')
    elif error == error_messages['no content']:
        test02_comments.append('[lookupcontent] -1% -- Unable to find content "hello world" with key: {}'.format(key_hello_world))
    test02_comments.extend(comments)

    # [+1%] Lookup content: unknown key
    new_key = str(int(key_hello) + int(key_hello_world))
    (content, error,comments) = lookup_content(peer1, new_key)

    if error == error_messages['no content']:
        test02_grade += 1
        test02_pos_grades.append('+1 for lookupcontent test')
    else:
        test02_comments.append('[lookupcontent] -1% -- Able to find content that should not exist with key: {}'.format(key_hello_world))
    test02_comments.extend(comments)
    #############################################################################

    #############################################################################
    #############################################################################
    # [+1%] remove content
    (content, error, comments) = remove_content(peer1, key_hello_world)

    if error == '' and len(comments) == 0 and content == 0:
        test02_grade += 1
        test02_pos_grades.append('+1 for removecontent test')
    if len(comments) != 0:
        test02_comments.extend(comments)
    if error == error_messages['no content'] or error == error_messages['no peer']:
        test02_comments.append('[removecontent] -1% -- Received error when trying to remove key: {}'.format(key_hello_world))

    # [+2%] Is the content actually removed: [Lookupcontent] 'hello world' --> error unknown key
    (content, error,comments) = lookup_content(peer1, key_hello_world)

    if error == error_messages['no content']:
        test02_grade += 2
        test02_pos_grades.append('+2 for removecontent test')
    else:
        test02_comments.append('[removecontent/lookupcontent] -2% -- Able to find content that was just removed with key: {}'.format(key_hello_world))

    # remove content
    remove_content(peer1, key_hello)

    # [+2%] Is the content actually removed: [Lookupcontent] 'hello' --> error unknown key
    (content, error,comments) = lookup_content(peer1, key_hello)

    if error == error_messages['no content']:
        test02_grade += 2
        test02_pos_grades.append('+2 for removecontent test')
    else:
        test02_comments.append('[removecontent/lookupcontent] -2% -- Able to find content that was just removed with key: {}'.format(key_hello_world))

    #############################################################################

    #############################################################################
    #############################################################################
    # [+2%] remove peer: peer1
    (content, error, comments) = remove_peer(peer1)

    if error == '' and content == '' and len(comments) == 0:
        test02_grade += 2
        test02_pos_grades.append('+2 for removepeer test')
    else:
        test02_comments.append('[removepeer] -2% -- Received error when trying to remove peer: {}'.format(peer1))
    #############################################################################

    
    #############################################################################
    #############################################################################
    # [+1%] Check if able to create a new network with one peer still in the network
    (peer1, comments) = new_network()

    # Check for errors; if errors then kill test 
    if peer1 is None or len(comments) != 0:
        # errors occurred
        test02_comments.extend(comments)
    else:
        if not '127.0.0.1' in peer1:
            test02_grade += 1
            test02_pos_grades.append('+1 for addpeer/2 simultaneous networks test')
        else:
            test02_comments.append('[addpeer/2 simultaneous networks] -1% -- Local ip address (127.0.0.1) returned')
    #############################################################################
    
    #############################################################################
    #############################################################################
    # [+3%] remove peer: peer2
    (content, error, comments) = remove_peer(peer2)

    if error == '' and content == '' and len(comments) == 0:
        test02_grade += 3
        test02_pos_grades.append('+3 for removepeer test')
    else:
        test02_comments.append('[removepeer] -3% -- Received error when trying to remove peer: {}'.format(peer1))
    #############################################################################
    

    return (test02_grade,test02_comments,test02_pos_grades)

if __name__ == '__main__':
    (gr,com, pos_grades) = test02()
    save_grade('test02', gr, test02_total_grade, com, pos_grades)

    print 'Grade: {} / {}'.format(gr, test02_total_grade)
    
    if gr == -1:
        print 'ERROR: an error occurred for test02.py!'
    elif gr != test02_total_grade:
        print 'You did not get perfect on this test, please review the following comments:'
    else:
        print 'You passed with full marks! Congratulations :)'
    
    if len(com) != 0:
        print 
        for c in com:
            print 'Grades Deducted:', c

    if len(pos_grades) != 0:
        print 
        for c in pos_grades:
            print 'Grades Received:', c
