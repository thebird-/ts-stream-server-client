#Test task for Streaming Engineer candidates

#1)	Implement multi-client TCP server console application for MPEG-2 Transport
# Streams broadcasting
#(main platform is Windows, but it will be a plus if application could be compiled
# and run on multiple platforms: Windows and Linux):
#Application must contain: DLL � server engine and console.
#Application must use MPEG-2 Transport Streams as multimedia content for the clients.
#Server must send MPEG-2 TS stream with correct data rate for given multimedia file;
#server must �detect� MPEG-2 TS files, and do not proceed with others (TS packet
# size is 188 bytes, sync byte is: 0x47); server must start data transfer from
# the first valid TS packet.
#	���� �� �� �������� � MPEG-2 ������������, ����� ��� ��� �������� ����������
# ��� ���� ���� 0x47 (����� ������ ��� ����� ������� �������� ���������� �����
# ��-������� ���� 5 ����� ���������, ���� ���� ��� ������� �� ���������, �.�. ������
# ���� ��������� ������ �� 0x47, ������ ������ �������� ������; ��� �� ����������
# MPEG-2 TS ������ � ������� ������ ������ �� 188 ����, � 192 ��� 204, � ����
# ������ ����� ���� ������ ������������ ��������� ��������� ������ �� ����������
# �������� � ��� ����� �������� ������ �� 188 ��������� ��������).
# Candidate could use one of the following methods to get bitrate information for the
# source file: 1) Put additional text file with pre-calculated bitrate inside, which
# server will read and use; 2) Parse MPEG-2 Transport Stream to extract all required
# data. Console application must be able to handle multiple TCP client connections
# simultaneously. That application must be developed in C++. STL/Boost should be used.
# Application must be multithread. Output log should be on screen and into file as well:
# current client count, total network channel usage, network channel usage per
# client � all these fields must be updated in real time.

#2)	Implement TCP client console application for dumping data into file (main
# platform is Windows, but it will be a plus if application could be compiled and
# run on multiple platforms: Windows and Linux):
#This is a client for server from the first task.
#Application must contain: DLL � client engine and console. Application must use
# command line for initialization (for example: client_app.exe tcp://10.11.11.11:1025).
# Application must handle wrong initialization settings correctly and show usage when
# necessary parameter is missing.
# Received file must be identical to the source one. Output log should be on screen only
# and contains the following information: current download speed
#(that value must be updated in real time during receiving), time spent for receiving,
# total bytes received.

