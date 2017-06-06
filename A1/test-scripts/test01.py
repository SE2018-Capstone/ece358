'''
Test Name : test01
Description : 
10% - compile and build: `test01.py`
* [1%] correctly package everything in the right folder
* [9%] `make` command operating correctly
    * -1% if makefile has obvious error
    * -1% some minor tweak of the makefile
'''

# pylint: disable=W0403
from helper import execute_make, check_files, save_grade

test01_total_grade = 10

def test01():
    '''
    Input : 
    Output : (grade, comments, pro_grades)
        grade=0-10 -- grade the student gets
        grade=-1 -- if an error occurred
    '''
    test01_grade = 0
    test01_comments = []
    test01_pos_grades = []

    (em_result, em_comments) = execute_make()

    if em_result == True:
        test01_grade += 9
        test01_pos_grades.append('+9 for no errors running make')
    else:
        test01_comments.append(em_comments)

    (cf_result, cf_comments) = check_files()
    if cf_result == True:
        test01_grade += 1
        test01_pos_grades.append('+1 for all files accurately created')
    else:
        test01_comments.append(cf_comments)

    return (test01_grade,test01_comments,test01_pos_grades)

if __name__ == '__main__':
    (grade01,comments01,pos_grades01) = test01()
    save_grade('test01', grade01, test01_total_grade, comments01, pos_grades01)

    print 'Grade: {} / {}'.format(grade01, test01_total_grade)
    
    if grade01 == -1:
        print 'ERROR: an error occurred for test02.py!'
    elif grade01 != test01_total_grade:
        print 'You did not get perfect on this test, please review the following comments:'
    else:
        print 'You passed! Congratulations :)'
    
    if len(comments01) != 0:
        print 
        for c in comments01:
            print 'Grades Deducted:', c

    if len(pos_grades01) != 0:
        print 
        for c in pos_grades01:
            print 'Grades Received:', c