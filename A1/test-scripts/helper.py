'''
File : helper.py
Description : this file holds common helper functions that are used in other scripts
'''

import random
import os.path, os
import csv

debug = True
executable_files = ['addpeer', 'removepeer', 'addcontent', 'removecontent', 
    'lookupcontent','allkeys']
grades_filename = 'grading.csv'
timeout_seconds = 120 # 2 minute timeout
# To test timeout functionality uncomment below
#timeout_seconds=1


def debug_msg(print_me):
    '''
    Description : A simple function to help print debugging statements.
                  You can hide all debugging statements by setting debug=False
    Input : print_me -- any object that should be printed
    Output : none
    '''
    if debug:
        print print_me

def list_to_string(ll, environment='itemize'):
    '''
    Description : Converts a list element into a Latex itemized list
    '''
    s = '\\begin{{{}}}'.format(environment) # \begin{itemize}
    for l in ll:
        s += '\\item[$\\bullet$] {}'.format(l)
    s += '\\end{{{}}}'.format(environment) # \end{itemize}
    s = s.replace('%', '\\%')
    return s


def save_grade(testname, grade, grade_total, negative_comments, positive_comments):
    '''
    Description : 
    Input : (testname, grade, grade_total, negative_comments, positive_comments)
        testname -- A string to indicate where the testing results came from
        grade -- An integer indicating the grade the student gets
        grade_total -- An integer indicating the maximum possible grade the student could receive
        negative_comments -- A list (can be empty) of strings that indicate where the student lost marks
        positive_comments -- A list (can be empty) of strings that indicate where the student gained marks
    Output : none
    '''
    include_header = False
    if os.path.isfile(grades_filename) == False:
        include_header = True

    with open(grades_filename, 'a') as csv_file:
        csv_writer = csv.writer(csv_file)

        if include_header:
            csv_writer.writerow(['Test Name', 'Student Grade', 
                'Total Grade', 'Grade Deductions', 'Grades Received'])
            csv_writer.writerow(['Extra Grades', '', '10', '', ''])
        if len(negative_comments) != 0:
            neg_str = list_to_string(negative_comments)
        else:
            neg_str = ''
        if len(positive_comments) != 0:
            pos_str = list_to_string(positive_comments)
        else:
            pos_str = ''
        csv_writer.writerow([testname, grade, grade_total, neg_str, pos_str])

def execute_command(cmd):
    '''
    Description : executes a command and returns: return code, output text, error text
    Input : cmd = A commandline string that will automatically split into a list
    Output : (return code, output, error, progam timeout)
        return code -- An Integer where 0 means no errors occurred (for well designed programs)
        output -- A string containing all output to stdout
        error -- A string containing all output to stderr
        program timeout -- Boolean where True means that command timedout
    '''
    returncode=-1
    prog_timeout=False
    stdout_file = 'stdout.{}.txt'.format(random.randint(1,10000))
    stderr_file = 'stderr.{}.txt'.format(random.randint(1,10000))

    #returncode = os.system('{} 1> {} 2> {}'.format(cmd, stdout_file, stderr_file))
    returncode = os.system('timeout {} {} 1> {} 2> {}'.format(timeout_seconds, cmd, stdout_file, stderr_file))

    if returncode == 31744:
        prog_timeout=True
        output=''
        error=''
    else:
        with open(stdout_file, 'r') as f:
            output = '\n'.join(f.readlines())
            output = output.strip()
        with open(stderr_file, 'r') as f:
            error = '\n'.join(f.readlines())
            error = error.strip()
    # Clean up files
    os.remove(stdout_file)
    os.remove(stderr_file)

    return (returncode, output, error, prog_timeout)

def is_int(s):
    '''
    Description : Checks if a string is an integer
    Input : A string
    Output : Returns TRUE if the string input is an integer, FALSE otherwise
    '''
    try: 
        int(s)
        return True
    except ValueError:
        return False

def check_files():
    '''
    Description : Checks that the proper commands are created
    Input : none
    Output : (result, comments)
        result = {True, False}
            True -- if all files exists and are executable
            False -- otherwise
        comments = {[], ['error comment', '...']}
    '''

    result = True
    comments = []

    for f in executable_files:
        # Check if each file exists
        if os.path.isfile(f) == False:
            result = False
            comments.append('[checkfiles] {} does not exists'.format(f))
        else:
            # Check if each file is executable
            if os.access(f, os.X_OK) == False:
                result = False
                comments.append('[checkfiles] {} is not executable'.format(f))
    return (result, comments)


def execute_make():
    '''
    Description : In the current working directory execute the `make` command and 
                  return any errors that `make` returns
    Input : none
    Output : (result, comments)
        result = {True, False}
            True -- if the make command returns with no errors
            False -- otherwise
        comments = {'', make_errors}
            '' -- if no errors occurred
            make_errors -- the error message that is returned by `make`
    '''
    result = False
    comments = ''

    (returncode, _, error,timeout) = execute_command('make')

    if returncode == 0 and error == '' and timeout == False:
        result = True
    elif timeout == True:
        comments = '[make] Error Running make: Timed out!'
    else:
        comments = '[make] Error Running make: {}'.format(error)

    return (result, comments)
