
import sys
import glob
import serial


def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result
ser = serial.Serial(serial_ports()[0])
ser.flushInput()
while True:
    try:
        ser_bytes = ser.readline()
        decoded_bytes = str(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
        print(decoded_bytes)
        commands=["All ok!","Need Backup","The Enemy is down","Opening fire","Fall Back","Evac Needed","G","H"]
        split=decoded_bytes.split()
        index=int(split[-1][0])
        print(index,"split")
        s=""
        for i in range(len(split)-1):
            s=s+split[i]
        s=s+commands[index]
        with open("status.txt","w") as f:
            f.write(s+"\n")
    except e:
        print(e)
        break