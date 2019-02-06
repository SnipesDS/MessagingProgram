# MessagingProgram
Simple messaging program.
This program uses sockets for different processes to talk to eachother by sending messages.
Multiple "clients" can connect at a time.
Messages can be sent to all or a specific client.

Quick start-up guide:

1.Start the Server 'main' before running any client 'main's.

2.To start client 'main', ./main localhost.

3.This should start up the client side and it will connect to the Sever program.
It will ask for a handle. This will be used to identify you by other clients when sending other messages.
Messages need to be of the form "Recipient:Message"
    If the message is sent to everyone ":Message"

Shutdown Client: type in "exit". (note: this will not work when entering a handle)

Shutdown Server: cancel process (Crtl+c)
