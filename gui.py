from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLabel, QScrollArea, QPlainTextEdit, QPushButton
from PyQt5.QtCore import QTimer, pyqtSlot
import serial.tools.list_ports
import time

class Window(QWidget):
    def __init__(self):
        super().__init__()
        self.title = "Arduino <3"
        self.left = 100
        self.top = 100
        self.width = 305
        self.height = 300
        self.setGeometry(self.left, self.top, self.width, self.height)

        self.layout = QVBoxLayout()
        self.setLayout(self.layout)
        self.arduino = None

        self.measurement = QLabel()
        self.measurement.setStyleSheet('background-color: yellow; font: 30pt Comic Sans MS')
        self.measurement.setText("-")
        self.config_info = QPlainTextEdit()
        self.config_set = QPlainTextEdit()
        self.config_info.setReadOnly(True)
        self.config_info.appendPlainText("")
        self.config_set.setFixedSize(300, 30)
        self.measurement.setFixedSize(300, 50)
        self.config_info.setFixedSize(300, 100)

        self.button = QPushButton("Send")
        self.button.clicked.connect(self.on_click)
        self.button.setEnabled(False)
        self.layout.addWidget(self.measurement)
        self.layout.addWidget(self.config_info)
        self.layout.addWidget(self.config_set)
        self.layout.addWidget(self.button)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.read_data_from_arduino)
        self.timer.start(1000)
        self.logs = []

    def on_click(self):
        text = self.config_set.toPlainText().encode()
        self.arduino.write(text)

    def com_service(self):
        arduino_COM = ""
        import serial.tools.list_ports

        devices = []
        while (len(devices) == 0):
            print("Looking for a device...")
            time.sleep(0.5)
            ports = list(serial.tools.list_ports.comports())
            devices = [p.device for p in ports if "Arduino" in p.description]
            if (devices):
                print("Found Arduino on {0}".format(devices[0]))
                arduino_COM = devices[0]

                self.arduino = serial.Serial(arduino_COM, 9600, timeout=0.1)
                self.button.setEnabled(True)


    def read_data_from_arduino(self):
        data = None
        if self.arduino == None:
            self.com_service()

        try:
            data = self.arduino.readline()
        except:
            self.config_info.appendPlainText("DEVICE DISCONNECTED\n")
            self.update()
            self.arduino = None
            self.button.setEnabled(False)

        if data:
            data = data.decode()
            if(data[0] == '!'): self.measurement.setText(data[1:]+" g")
            else:
                self.config_info.appendPlainText(data)
            self.update()
            data = None

