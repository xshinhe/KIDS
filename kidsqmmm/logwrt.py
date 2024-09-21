#! /usr/bin/env python3
# coding=utf-8

#    COBRAMM
#    Copyright (c) 2019 ALMA MATER STUDIORUM - Università di Bologna

#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#####################################################################################################

# import statements of module from python standard library

import sys  # system commands
import time  # provides various time-related functions
import random  # random number generator

# import cobramm local module

import qmmmenv

# imports of local objects

from Timer import Timer  # keep timings of the different sections of the code

# math libraries

import numpy as np  # numpy library for scientific computation

#####################################################################################################
# module global variables
#####################################################################################################

# variable that controls the cleaning up of the non-essential COBRAMM files at the end of the run
DEBUG_COBRAMM_RUN = False

# set to True to write output to std out, otherwise write output to "cobram.log" file
writeLogToStdOut = True
# log verbosity level
VERBOSITY_LEVEL = 0


#####################################################################################################
# log, warning and error functions
#####################################################################################################


def setverbositylevel(levelnumber):
    """ Set the level of verbosity of the log file, by changing the value of the
    value VERBOSITY_LEVEL that is used in this module.

    :param levelnumber: an integer number (0 = minimal output, >0 increasing amount of output)
    """

    # use and modify the global variable of this module
    global VERBOSITY_LEVEL
    # set the level equal to the input number
    VERBOSITY_LEVEL = int(levelnumber)


# ================================================================================

def writelog(message, level=0):
    """ Write message to log (either standard output or cobram.log file).
       Take as input the string that contains the message to print. """

    if level <= VERBOSITY_LEVEL:
        if writeLogToStdOut:
            # write a string to standard output
            sys.stdout.write(message)
        else:
            # write a string in the cobram.log file
            with open('cobramm.log', 'a') as log:
                log.write(message)
    return


# ================================================================================

def printGeom(geometry):
    """ print to log file information on the molecular geometry """

    if geometry.atomNum > 500:
        writelog("Only HM atom coordinates will be printed here\n\n")
        atoms = zip(geometry.list_MEDIUM_HIGH, geometry.getAtomLabels("MEDIUM_HIGH"),
                    *geometry.getModel("MEDIUM_HIGH"))
    else:
        atoms = zip(range(1, geometry.atomNum + 1), geometry.atomLabel, *geometry.cartesian)

    # Molecular geometry
    writelog("""----------------------------------------------------------------
   Atom     Atomic              Coordinates (Angstroms)
     ID      Label             X           Y           Z
 ----------------------------------------------------------------
""")
    for at in atoms:
        writelog(" {0:6d} {1:>10s}     {2:12.6f}{3:12.6f}{4:12.6f}\n".format(*at))
    writelog("----------------------------------------------------------------\n\n")
    writelog("\n")
    return


# ================================================================================

def printLayers(listH, listM, listL, listBA):
    """ print to log file information on the H-M-L layers and the link atoms """

    # High layer atoms
    if len(listH) > 0:
        writelog(" * {} high layer atom(s): ".format(len(listH)))
        writelog(prettyAtomsList(listH) + "\n")
    else:
        writelog(" * no medium layer atom \n")

    # Medium layer atoms
    if len(listM) > 0:
        writelog(" * {} medium layer atom(s): ".format(len(listM)))
        writelog(prettyAtomsList(listM) + "\n")
    else:
        writelog(" * no medium layer atom \n")

    # Low layer atoms
    if len(listL) > 0:
        writelog(" * {} low layer atom(s): ".format(len(listL)))
        writelog(prettyAtomsList(listL) + "\n")
    else:
        writelog(" * no low layer atom \n")

    # Links
    if len(listBA[0]) > 0:
        writelog(" * atom links (QM --> MM): ")
        for A, B in zip(*listBA):
            writelog("{} --> {}   ".format(B, A))
        writelog("\n")

    writelog("\n")
    return


# ================================================================================

def prettyAtomsList(atomList):
    """ return a compact string that defines the atoms indices contained in the input atomList list:
        atoms that have index with consecutive integer values are written in the N1-N2 form """

    # initialize string to store formatted list of atoms
    string = ""

    # initialize integers that store the start of a sequence of numbers and the number preceeding the current one
    seqStart = None
    seqPre = None

    # loop over the atoms of the list
    for atom in atomList:

        if seqPre is None:  # in this case atom is the first element
            seqStart = atom

        else:  # in this other case, we are beyond the first element
            if seqPre != atom - 1:  # if the sequence is interrupted, write something to close the open sequence
                if seqStart != seqPre:  # when the start and the preceeding elements are different, this is an interval
                    string += " {0}-{1}".format(seqStart, seqPre)
                else:  # otherwise this is an alone number
                    string += " {0}".format(seqStart)
                seqStart = atom  # restart the sequence

        # store actual element for the next iteration
        seqPre = atom

    # close the final sequence
    if seqStart != atomList[-1]:
        string += " {0}-{1}".format(seqStart, atomList[-1])
    else:
        string += " {0}".format(atomList[-1])

    return string


# ================================================================================


def printEnergies(modelH_QM, modelH_MM, real_MM, only_MM, chargeInt, full_QMMM, modelLabel):
    """ print to log file information on the QM/MM energy and its components """

    if modelLabel == "M" or modelLabel == "ML":
        writelog(' ' * 20 + 'Energies (Hartrees)\n')
        writelog('   MM energy:  ' + '%18.8f\n' % only_MM)

    else:
        for i, E_QM, E_QMMM in zip(range(len(modelH_QM)), modelH_QM, full_QMMM):
            writelog('-' * 60 + "\n")
            writelog(' ' * 20 + 'STATE %3d' % (i + 1) + "\n")
            writelog('-' * 60 + "\n")
            writelog(' ' * 20 + 'Energies (Hartrees)\n')
            writelog('   Model-H QM:  {0:18.8f}\n'.format(E_QM) +
                     '   Real MM:     {0:18.8f}\n'.format(real_MM) +
                     '   Model-H MM:  {0:18.8f}\n'.format(modelH_MM) +
                     '   Emb-emb crg: {0:18.8f}\n'.format(chargeInt) +
                     '   QM Energy:   {0:18.8f}\n\n'.format(E_QM - chargeInt) + '\n' +
                     'E(tot)=E(Model-H QM)+(Real MM)-(Model-H MM)-(Emb-emb)= {0:18.8f}\n\n'.format(E_QMMM))

    return


# ================================================================================


def printModelHCharges(geometry, charges, dipole):
    """ print to log file information on the charges of the model-H QM
    calculation and the dipole moment """

    # Molecular geometry
    writelog(""" ----------------------------------------------
    Atom     Atomic       Model+H Charges
      ID      Label           (a.u.)
 ----------------------------------------------
""")
    labelModelH = [geometry.atomLabel[i - 1] for i in geometry.list_QM]
    for i in range(geometry.NsubH):
        labelModelH.append('H')
    for n in range(len(labelModelH)):
        writelog("  {0:6d} {1:>10s}     {2:12.6f}\n".format(n + 1, labelModelH[n], charges[n]))
    writelog(" ----------------------------------------------\n")
    writelog("         {0:>10s}     {1:12.6f}\n".format("TOTAL", sum(charges)))
    writelog(" ----------------------------------------------\n\n")

    kconv_au_Deb = 0.39342215569939517797

    writelog(""" ----------------------------------------------
                         Dipole Moment
                     (a.u.)         (Debye)
 ----------------------------------------------
""")
    componentsLab = ["X", "Y", "Z", "magnitude"]
    for n in range(3):
        writelog("  {0:>10s}  {1:14.6f} {2:14.6f}\n".format(componentsLab[n], dipole[n], dipole[n] / kconv_au_Deb))
    writelog(" ----------------------------------------------\n")
    writelog("  {0:>10s}  {1:14.6f} {2:14.6f}\n".format(componentsLab[3], dipole[3], dipole[3] / kconv_au_Deb))
    writelog(" ----------------------------------------------\n\n")

    return


# ================================================================================

def printMDinfo(actualTime, state, tStep, mdItems, potReference):
    writelog("{0:<20s} {1:f} fs\n\n".format("Simulation time:", actualTime / 41.341373337))
    writelog("{0:<30s} {1:d}\n".format("Current electronic state:", state + 1))
    writelog("{0:<30s} {1:f} fs\n\n".format("Velocity-Verlet time-step:", tStep / 41.341373337))
    writelog("{0:<20s} {1:f} eV\n".format("Kinetic energy:", mdItems[2] * 27.211385))
    writelog("{0:<20s} {1:f} eV\n".format("Potential energy:", (mdItems[1] - potReference) * 27.211385))
    writelog("{0:<20s} {1:f} eV\n".format("Total energy:", (mdItems[0] - potReference) * 27.211385))
    writelog("\n")


# ================================================================================

def startSection(sectionName):
    """ Write to log the title of the section that is started in a specific point
       of the code. The name of the section is given as input argument. """

    titleLen = len(sectionName)
    blankSpaces = int((80 - titleLen) / 2)

    output = ('=' * 80 + '\n' +
              ' ' * blankSpaces + sectionName + '\n' +
              '-' * 80 + '\n\n')
    writelog(output)


# ================================================================================

def startSubSection(subSectionName):
    """ Write to log the title of the subsection that is started in a specific point
       of the code. The name of the subsection is given as input argument. """

    titleLen = len(subSectionName)
    blankSpaces = int((80 - titleLen) / 2)

    output = (' ' * blankSpaces + "*" * titleLen + "\n" +
              ' ' * blankSpaces + subSectionName + '\n' +
              ' ' * blankSpaces + "*" * titleLen + "\n\n")
    writelog(output)


# ================================================================================

def writewarning(message):
    """ Print a warning message to log.
       Take as input the string that contains the warining message to print. """

    writelog('\nWARNING! {0}\n'.format(message))


# ================================================================================

def fatalerror(message):
    """ Write an error message end exit from COBRAMM.
       Take as input the string that contains the error message to print.
       Break COBRAM execution with system status equal to 3. """

    startSection("CALCULATION TIMES")
    Timer.timingreport(writelog)

    # print error message to log
    writelog('FATAL:  {0}\n\n'.format(message) +
             '=' * 80 + '\n' +
             'Ending time: {0}\n'.format(time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())) +
             '=' * 80 + '\n\n' +
             '--- COBRAM calculation ABORTED!!! ---\n')

    # exit with status 3
    sys.exit(3)


#####################################################################################################
# start and end of COBRAMM
#####################################################################################################

def cobramstart():
    writelog("=" * 115 + "\n\n" \
"               ######  #####                                                                                    \n" \
"              # #  ####  *  #                                                                                   \n" \
"              # #  *   #####                                                                                    \n" \
"              # #     ############                                                                              \n" \
"              ## ####        ## ###                                                                             \n" \
"          #### ##                 #                                                                             \n" \
"        ## #                      #                                                                             \n" \
"       #  ##                      #                                                                             \n" \
"      # ######                   #                                                                              \n" \
"     ##### ##  ###            ###__ __                                                                          \n" \
"    #     ##     ############      #  ###                                                                       \n" \
"   #############___   #######      #__   #                                                                      \n" \
"   #     ##    ##  ###   ##########   ###   ::.      .:.       ..             :.     :        :.    ..       .: \n" \
"   #######    ##    #####                 -#****.  .*****#   ******#         #*+    #**      #**   .**.     .** \n" \
"   #    ##   ##    ###                  +********.  +*+ **=  ***: +**       +**:   .***     #**=   +***    .*** \n" \
"   #__###########__#                   ***-    **:  **: **=  -*#  .*#      +***.   +*#*+   #***:  .****   .**** \n" \
"    ## ##    #   ##                    **=    .**: :*****+   **=  **=     +***#    **:**  ***#*:  -****+ .**:** \n" \
"     # #     #  ##                    #**.    ***. *******  .******=     +**+*#   =** #*.+** #*.  **:.**.**-.** \n" \
"     ###########                      #*%     **#  **.  *** ****:    .*****+**#.  #*. -*#**  #*  :**  #***- .** \n" \
"    ###     # #                       #*%    ***. =**   =*# **#*-    :****#****# -**   ***   **  #*=  :**=  :*# \n" \
"   #  #     ##                        ***. .***..=**: =***.-**=*#   .#**-   .**: **:   .-    ** .**    =:   :*# \n" \
"  ###########               ___  __    #*****+  -#****#+.  **+ ***.***=      **+ ##          ## -*+         .** \n" \
" #   ##     #   ____########   #   ##_                          **#.       .**.                                 \n" \
" #   ##      ###____#______#####_____ ###                        .#**#+++#**#.                                  \n" \
" ###### _  __##    #    ##      #     # #                           .+###+.                                     \n" \
"  #_  #  ##       #      #      #     ##                                                                        \n" \
"    #####__      #      ##    /######                                                                           \n" \
"           ###################                                                                                  \n\n" + "=" * 115+"\n")
    # start the log with cobramm-like message
    writelog('\n' + 'COBRAMM'.center(115) + "\n")
    # print initial cobramm-like message to log
    writelog(randomCobrammAcronym())

    # get last git tag and date of the last commit
    cobramm_version = qmmmenv.getVersion()
    # get hostname
    hostname = qmmmenv.getHostname()

    # write the specific description of this tool
    writelog("The cobram QMMM interface is made by:".center(115) + "\n" +
             "Piero Altoe' and Marco Stenta".center(115) + "\n" +
             "Universita' degli Studi di Bologna, 2007".center(115) + "\n\n" +
             "Updates & extensions by".center(115) + "\n" +
             "Oliver Weingart".center(115) + "\n" +
             "Heinrich-Heine-Universitaet Duesseldorf".center(115) + "\n\n" +
             "Artur Nenov, Matteo Bonfanti, Davide Avagliano and Flavia Aleotti".center(115) + "\n" +
             "Universita' degli Studi di Bologna".center(115) + "\n" +
             "Version {0}".format(cobramm_version).center(115) + "\n\n" +
             '=' * 115 + '\n' +
             'Starting time: ' + time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime()) + '\n' +
             '=' * 115 + '\n\n' +
             'Hostname: {0}\n\n'.format(hostname))


# ================================================================================

def randomCobrammAcronym():
    # define the list of alternative acronym definitions available
    acrolist = [
        ['COBRAMM',
         '^^^^^^^'],
        ['Computational BRidge between Ab-initio and Molecular Mechanics',
         '^             ^^             ^             ^         ^        '],
        ['Computations OBtained by Running Ab-initio and Molecular Mechanics',
         '^            ^^          ^       ^             ^        ^         '],
        ['Calculations OBtained by Running Ab-initio and Molecular Mechanics',
         '^            ^^          ^       ^             ^         ^        '],
        ['Comprehensive OBservation of the Reality: from Ab-initio to Molecular Mechanics',
         '^             ^^                 ^             ^            ^         ^        '],
        ['Complete OBservation of the Reality: from Ab-initio to Molecular Mechanics',
         '^        ^^                 ^             ^            ^         ^        '],
        ['Combined OBservation of the Reality with Ab-initio and Molecular Mechanics',
         '^        ^^                 ^            ^             ^         ^        '],
        ['Cobram is Optimized in Bologna to Run Ab-initio and Molecular Mechanics',
         '^         ^            ^          ^   ^             ^         ^        '],
        ['Cobram can be Obtained from Bologna to Run Ab-initio and Molecular Mechanics',
         '^             ^             ^          ^   ^             ^         ^        '],
        ['CObram was Born to Run Ab-initio and Molecular Mechanics',
         '^^         ^       ^   ^             ^         ^        ']
    ]

    # randomly choose one of the acronyms
    index = random.randint(0, len(acrolist) - 1)
    # format the acronym in a string
    #OLD FORMATTING
    #acronymString = '\n            {0}\n             {1}\n\n'.format(*acrolist[index])
    #NEW FORMATTING
    acronymString = "\n" + acrolist[index][0].center(115) + "\n" + acrolist[index][1].center(115) + "\n\n"

    # return the string
    return acronymString


# ================================================================================

def cobramend():
    """ write the final message and terminate COBRAMM execution normally,
        exit status is equal to 0 """

    startSection("CALCULATION TIMES")
    Timer.timingreport(writelog)

    # print final message to log
    writelog('\n' +
             '=' * 80 + '\n' +
             'Ending time: ' + time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime()) + '\n' +
             '=' * 80 + '\n\n' +
             '--- COBRAM calculation terminated normally ---\n')

    # exit with status 0
    sys.exit(0)


# ===============================================================================================================

def matrix_prettystring(matrix: np.ndarray, fmt: str = ".6f", atomLabels=None) -> str:
    """
    Return a string with a nice formatted representation of a matrix, stored in a ndarray numpy object.
    The optional argument fmt should contain a string that defines the numeric format of the
    data to represent (without the field size that will be determined dinamically.

    :param matrix: numpy ndarray with the matrix to print to string
    :param fmt: numerical format to use in the matrix representation
    :return: string that contains the representation of the matrix
    """
    # compute the size that is needed to represent each column of the matrix
    colmax = [max([len(("{:" + fmt + "}").format(x))+1 for x in col]) for col in matrix.T]
    # construct the format of the line
    rowformat = "".join(["{:" + str(colsize) + fmt + "}" for colsize in colmax])
    # print each row with the appropriate format and store the result in a string
    if atomLabels is None:
        string = "\n".join([rowformat.format(*row) for row in matrix]) + "\n"
    else:
        string = "\n".join([("{:4s}"+rowformat).format(atomLabels[irow],*matrix[irow]) for irow in range(len(matrix))]) + "\n" 
    # return the string with the matrix representation
    return string

#####################################################################################################
