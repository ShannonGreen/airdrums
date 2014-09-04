################################################################################
# serialToMidi.py
#
# take in serial data in the form of midi information and output as midi
# 
# Lauren Spooner 4/9/14
################################################################################

import sys, serial
import pygame.midi

BAUDRATE = 115200
NOTE_ON = 153
NOTE_OFF = 137

# main() function
def main():
    # expects 2 arguments - serial port string, midi output number
    if(len(sys.argv) != 3):
        print 'Incorrect number of arguments specified'
        print 'Defaulting to reading from stdin and outputing to midi0'
        print 'Usage:', sys.argv[0], '<serial port> <midi output number>\n'
        
        ser = sys.stdin
        midiOutput = 0
    else: 
        strPort = sys.argv[1]
        ser = serial.Serial(strPort, BAUDRATE)
        midiOutput = int(sys.argv[2])

    pygame.midi.init()
    player = pygame.midi.Output(midiOutput)

    while True:
        try:
            # expecting 3 numbers, space separated:
            # first number specifies note on, 153, or note off, 137
            # second number is the note, middle C is 74 I believe
            # 3rd number is the velocity, 127 is the max apparently
            #TODO: I think our velocity goes higher than this at the moment, so for now I'm just going to halve it to make sure it's in the right range, should possibly fix that in the code
            line = ser.readline()
            data = [int(val) for val in line.split()]
            
            if len(data) == 3:
                playNote(player, data)
            else:
                print "incorrect number of values read, can't play note"
                print "data read", data
            
        except KeyboardInterrupt:
            print 'Exiting'
            break
        
    del player
    pygame.midi.quit()

    ser.flush()
    ser.close()

def playNote(player, data):
    if data[0] == NOTE_ON:
        player.note_on(data[1], data[2]/2)
    elif data[0] == NOTE_OFF:
        player.note_off(data[1], data[2]/2)
    else:
        print "incorrect id byte recieved (" + str(data[0]) + "), can't play note"
        print "should be " + str(NOTE_ON) + ", note on, or " + str(NOTE_OFF) + ", note off"
    
if __name__ == '__main__':
    main()
