'''
File : common_tasks.py
Description : This file holds all operations that directly deal with common tasks that are 
              performed within the student project's network. 
              Such common tasks include:
                 * Start a new network
                 * Add a peer to the network
                 * Add content to the network
                 * Retrieve content from the network
'''

# pylint: disable=W0403
from helper import execute_command, is_int


error_messages = {
    'no peer':'Error: no such peer',
    'no content':'Error: no such content'
}

def new_network():
    '''
    Description : Starts a new network and returns the ip address and port for the new peer
    Input : none
    Output : (peer, comments)
        peer = {None, 'ip port'}
        comments = {[], ['error comment 1', ...]}
    '''
    return add_peer('')

def add_peer(ip_port):
    '''
    Description : Adds a peer to an existing network 
                  or starts a new network if ip_port is an empty string
    Input : ip_port -- a string containing the ip address and port: 'ip port'
    Output : (peer, comments)
        peer = {None, 'ip port'}
            None -- this means an error occurred and no peer was added to the network
            'ip port' -- this is the ip address and port number of the newly added peer
        comments = {[], ['error comment 1', ...]}
            [] -- this means no errors occurred
            ['.', ''] -- A list of error messages
    '''
    peer = None
    comments = []

    (rc, out, err,timeout) = execute_command('./addpeer {}'.format(ip_port))

    if rc != 0:
        comments.append('[addpeer] Return Code is: {}'.format(rc))
    if err != '':
        comments.append('[addpeer] Error String: {}'.format(err))
    if timeout != False:
        comments.append('[addpeer] Timed Out!')
        return (peer,comments)
    
    #################
    # Checking for proper output format: ip_address port
    sp = out.split()
    if len(sp) != 2:
        comments.append('[addpeer] Invalid return string: {}; expecting 2 elements but there are {} elements'.format(out, len(sp)))
    else:
        ip = sp[0]
        port = sp[1]

        ip_sp = ip.split('.')

        if len(ip_sp) != 4:
            comments.append('[addpeer] Invalid IP returned: {}'.format(ip))
        else:
            # debug_msg(ip_sp)
            if not is_int(port) or not is_int(ip_sp[0]) or not is_int(ip_sp[1]) or not is_int(ip_sp[2]) or not is_int(ip_sp[3]):
                comments.append('[addpeer] IP and Port must be integers {} {}'.format(ip, port))

    # Checking for any errors found above
    if len(comments) == 0:
        # No errors
        peer = out

    return (peer, comments)

def add_content(ip_port, content):
    '''
    Description : Adds content to an existing network
    Input : ip_port, content
        ip_port -- a string containing the ip address and port: 'ip port'
        content -- a string containing some content to be added to the network
    Output : (key, comments)
        key = {None, 'int'}
            None -- this means an error occurred and no peer was added to the network
            'int' -- this is the unique key given for that content
        comments = {[], ['error comment 1', ...]}
            [] -- this means no errors occurred
            ['.', ''] -- A list of error messages
    '''
    key = None
    comments = []

    (rc, out, err,timeout) = execute_command('./addcontent {} "{}"'.format(ip_port, content))

    if rc != 0:
        comments.append('[addcontent] Return Code is: {}'.format(rc))
    if err != '':
        comments.append('[addcontent] Error String: {}'.format(err))
    if timeout != False:
        comments.append('[addcontent] Timed Out!')
        return (key,comments)

    if not is_int(out):
        comments.append('[addcontent] Output string must be a integer key: {}'.format(out))
    else:
        # No errors
        key = out

    return (key, comments)
    
def all_keys(ip_port):
    '''
    Description : Returns all keys stored on a peer
    Input : ip_port
        ip_port -- a string containing the ip address and port: 'ip port'
    Output : (keys, comments)
        key = {None, [], ['key1', ...]}
            None -- this means an error occurred and no peer was added to the network
            [] -- this means that no content is stored on this peer
            ['key1', ...] -- this is the list of content that is stored on this peer
        comments = {[], ['error comment 1', ...]}
            [] -- this means no errors occurred
            ['.', ''] -- A list of error messages
    '''
    keys = None
    comments = []

    (rc, out, err,timeout) = execute_command('./allkeys {}'.format(ip_port))

    if rc != 0:
        comments.append('[allkeys] Return Code is: {}'.format(rc))
    if err != '':
        comments.append('[allkeys] Error String: {}'.format(err))
    if timeout != False:
        comments.append('[allkeys] Timed Out!')
        return (keys,comments)

    if out == '':
        keys = []
    else:
        sp = out.split()

        passed_test = True
        for s in sp:
            if not is_int(s):
                passed_test = False
                break
        if not passed_test:
            comments.append('[allkeys] All keys must be integers: {}'.format(out))
        else:
            keys = sp


    return (keys, comments)

def lookup_content(ip_port, key):
    '''
    Description : Returns the content stored for a specific key or the Error message
                  'Error: no such content' if there is no content
    Input : ip_port, key
        ip_port -- a string containing the ip address and port: 'ip port'
        key -- a string that holds an integer key
    Output : (content,error,comments)
        content -- a string for what is returned on stdout
        error -- a string for what is returned on stderr
        comments = {[], ['error comment 1', ...]}
            [] -- this means no errors occurred
            ['.', ''] -- A list of error messages
    '''
    comments = []

    (rc, content, error,timeout) = execute_command('./lookupcontent {} {}'.format(ip_port,key))

    if rc != 0:
        comments.append('[lookupcontent] Return Code is: {}'.format(rc))
    if error != '':
        if error != 'Error: no such content' and error != 'Error: no such peer':
            comments.append('[lookupcontent] Unknown Error String: {}'.format(error))
    if timeout != False:
        comments.append('[lookupcontent] Timed Out!')
        content=''
        error=''

    return (content,error,comments)

def remove_content(ip_port, key):
    '''
    Description : Removes the content stored for a specific key. 
                  Should have no output on stdout, might have errors on stderr.
    Input : ip_port, key
        ip_port -- a string containing the ip address and port: 'ip port'
        key -- a string that holds an integer key
    Output : (content,error,comments)
        content -- a string for what is returned on stdout
        error -- a string for what is returned on stderr
        comments = {[], ['error comment 1', ...]}
            [] -- this means no errors occurred
            ['.', ''] -- A list of error messages
    '''
    comments = []

    (rc, content, error,timeout) = execute_command('./removecontent {} {}'.format(ip_port,key))

    if rc != 0:
        comments.append('[removecontent] Return Code is: {}'.format(rc))
    if error != '':
        if error != error_messages['no content'] and error != error_messages['no peer']:
            comments.append('[removecontent] Unknown Error String: {}'.format(error))
    if timeout != False:
        comments.append('[removecontent] Timed Out!')
        content=''
        error=''

    return (content,error,comments)

def remove_peer(ip_port):
    '''
    Description : Removes a peer and returns any output on stdout and stderr
    Input : ip_port
        ip_port -- a string containing the ip address and port: 'ip port'
    Output : (stdout,error,comments)
        stdout -- a string for what is returned on stdout
        error -- a string for what is returned on stderr
        comments = {[], ['error comment 1', ...]}
            [] -- this means no errors occurred
            ['.', ''] -- A list of error messages
    '''
    comments = []

    (rc, stdout, error, timeout) = execute_command('./removepeer {}'.format(ip_port))

    if rc != 0:
        comments.append('[removepeer] Return Code is: {}'.format(rc))
    if error != 'Error: no such peer' and error != '':
        comments.append('[removepeer] Unknown Error String: {}'.format(error))
    if stdout != '':
        comments.append('[removepeer] There should be no output on stdout from removepeer: {}'.format(stdout))
    if timeout != False:
        comments.append('[removepeer] Timed Out!')
        stdout=''
        error=''


    return (stdout,error,comments)
