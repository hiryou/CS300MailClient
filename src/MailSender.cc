/**
 * Copyright 2010 by Long Nguyen
 * Please included license in COPYING
 * 	or visit http://opensource.org/licenses/mit-license.php
 */

/**
 * MailSender.cc
 * Mail Sender Class Implementation
 */

#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include "MailSender.hh"

/**
 * MailSender Constructor
 * 	- input RFC-822 email file
 * 	- config mail host and corresponding port, default is mailhost.cs.pdx.edu:25
 */
MailSender::MailSender(string &filename) {
	// indicate success status
	status = 0;
	// get filename
	emailFile = filename;
	
	// ifstream to read config file
	string configFilePath("email.conf");
    ifstream configFile(configFilePath.c_str(), ifstream::in);
    
    // if config file exists
    if (configFile) {
		// string to store config info
		string config;
        // get content of config file, line by line
        string line;
        while (getline(configFile, line) != 0) {
            config += line + "\n";
        }
        configFile.close();
        
        // look up config
        size_t lookup;
        // look up mailhost from config
        if ((lookup = config.find("MAILHOST=")) != string::npos) {
            // retrieve host
            mailhost = config.substr(lookup + 9);
            mailhost = trim(mailhost);
        } else {
            // exit due to corrupted config file.
            cout 	<<"Config file " <<configFilePath
					<<" was corrupted: Can not find MAILHOST.\n";
			status = 1;
            return;
        }
        
        // look up connection port from config
        if ((lookup = config.find("PORT=")) != string::npos) {
            // retrieve host
            port = config.substr(lookup + 5);
            port = trim(port);
        } else {
            // exit due to corrupted config file.
            cout 	<<"Config file " <<configFilePath
					<<" was corrupted: Can not find PORT.\n";
			status = 1;
            return;
        }
    // else: it does not exist
    } else {
        // use default mailhost
        mailhost 	= MailSender::DEFAULT_MAILHOST;
        port		= MailSender::DEFAULT_PORT;
    }
}

const std::string MailSender::COMMAND_HELP 		="help";
const std::string MailSender::COMMAND_INSTALL 	="install";
const std::string MailSender::COMMAND_README 	="readme";
const std::string MailSender::COMMAND_LICENSE 	="license";

const std::string MailSender::DEFAULT_MAILHOST 	="mailhost.cs.pdx.edu";
const std::string MailSender::DEFAULT_PORT 		="25";

/**
 * MailSender Process
 * This function is designated to parse the emailFile, it will extract:
 * 	- FROM Address
 * 	- TO Address
 * 	- Message content including Subject and Body
 */
int MailSender::process() {
	// execute iff status = 0
	if (status == 0) {
		// ifstream to read rfc-822 formatted email file
		ifstream fp(emailFile.c_str());
		// exit if email file does not exist
		if (!fp) {
			cout <<"Email file " <<emailFile <<" is not found!\n";
			return 1;
		}
		
		cout <<"Opening email file " <<emailFile <<"..." <<endl;
		// get content of email file, line by line
		mailContent = "";
		string line;
		while (getline(fp, line) != 0) {
			// SendIt's data member building up its data line by line
			// Modify standalone "." to ensure message isn't cut off
			if (line.compare(".") == 0)
				line = "..";
			mailContent += line + "\n";
		}
		fp.close();

		// find FROM address from the mail content
		string envFrom = getMailElement(mailContent, "From:");
		// Check for error from find
		if (envFrom.empty() == true) {
			cout << "From field is not specified.\n";
			return 1;
		}
		
		// find TO address from the mail content
		string envTo = getMailElement(mailContent, "To:");
		// Check for error from find
		if (envTo.empty() == true) {
			cout << "To field is not specified.\n";
			return 1;
		}
		
		// prepare to deliver mail
		cout <<"Prepare to deliver...\n";
		cout <<"MAIL FROM: <" <<envFrom <<">" <<endl;
		cout <<"MAIL TO: <" <<envTo <<">" <<endl;
		cout <<mailContent <<endl;
		cout <<"Processing..." <<endl;

		// now send it!
		if (deliver(mailhost, envFrom, envTo) != 0) {
			cerr << "Error sending message. Please try again.\n";
			return 1;
		}
		
		// return 0 in success
		return 0;
	}
}

/**
 * MailSender Get Mail Element
 * 	Extract Email address (identifier@host.ext) from a given mail content
 */
string MailSender::getMailElement(string &msg, string target) {
	// try to extract the line containing target
	string targetLine;
	// start pos is the index of the substring target
    size_t startPos = msg.find(target);
    // if there is no targetLine
    if (startPos == string::npos) {
		// return empty string for error indication
		return "";
	}
    // end pos is the index of the following "\n" or the end of the msg
    size_t endPos;
    if (msg.find("\n", startPos) > 0)
		endPos = msg.find("\n", startPos);
	else
		endPos = msg.length();
	// extract the target line
	targetLine = msg.substr(startPos, endPos - startPos + 1);
	
	// find the '@' character on this line
	size_t at = targetLine.find("@");
	// if no '@' is found
	if (at == string::npos) {
		// return empty string for error indication
		return "";
	}
	
	// loop from at to the left of targetLine to find the initial point of the target
	int i;
	for (i=at; i>=0; i--)
		if (targetLine[i] == ' ' || targetLine[i] == '<' || targetLine[i] == '\r' || targetLine[i] == '\n' || targetLine[i] == '\t') {
			startPos = i + 1;
			break;
		}
	// loop from at to the right of targetLine to find the end point of the target
	for (i=at; i<targetLine.length(); i++)
		if (targetLine[i] == ' ' || targetLine[i] == '>' || targetLine[i] == '\r' || targetLine[i] == '\n' || targetLine[i] == '\t') {
			endPos = i - 1;
			break;
		}
	
	// find and trim element
	string element = targetLine.substr(startPos, endPos - startPos + 1);
	element = trim(element);
	
	// return
	return element;
}

/**
 * Trim a string, remove "\t", "\r", "\n", " " surrouding the given string
 */
string MailSender::trim(string &str) {
	size_t lookup;
	string result(str);
    // trim whitespaces
    while ((lookup = result.find(" ")) != string::npos) {
        result.erase(lookup);
    }
    // trim tabs
    while ((lookup = result.find("\t")) != string::npos) {
        result.erase(lookup);
    }
    // trim \r
    while ((lookup = result.find("\r")) != string::npos) {
        result.erase(lookup);
    }
    // trim new line char
    while ((lookup = result.find("\n")) != string::npos) {
        result.erase(lookup);
    }
    
    return result;
}

/**
 * MailSender Delivery Process
 *	Connect to mailhost and attempt to deliver the email
 */
int MailSender::deliver(string &host_to, string &envelope_from, string &envelope_to) {
	int socketDescriptor;
	string hostTo(host_to);
	string smtpMsg, response, resultCode;
	
	int status;							// status resulted from connection
	struct addrinfo hints;
	struct addrinfo *servInfo;  		// will point to the results

	memset(&hints, 0, sizeof hints); 	// make sure the struct is empty
	hints.ai_family 	= AF_UNSPEC;    // don't care IPv4 or IPv6
	hints.ai_socktype 	= SOCK_STREAM; 	// TCP stream sockets
	hints.ai_flags 		= AI_PASSIVE;   // fill in my IP for me
	
	// try to connect to mailhost
	if ((status = getaddrinfo(host_to.c_str(), port.c_str(), &hints, &servInfo)) != 0) {
		// log error No
		errno = status;
		// print out error and return -1 if fail
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}
	
	// get the socket descriptor
    socketDescriptor = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
    // connect to the socket
    status = connect(socketDescriptor, servInfo->ai_addr, servInfo->ai_addrlen);
    // return fail if can not connect
    if (status != 0) {
		// log error No
		errno = status;
		// print out error and return -1 if fail
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}
    
    // initialize EHLO message
    smtpMsg = "EHLO ";
    smtpMsg += host_to + "\r\n";
    // contact server
    response = sendRecv(socketDescriptor, smtpMsg);
    resultCode = response.substr(0, 3);
    // stop if the server refuse connection
    if (resultCode.compare("220") != 0) {
		// log error NO
		errno = resultCode;
		// print out error and return -1 if fail
        cerr <<response;
        close(socketDescriptor);
        freeaddrinfo(servInfo);
        return -1;
    }
    
    // set MAIL FROM
    smtpMsg = "MAIL FROM: <" + envelope_from + ">";
    smtpMsg += "\r\n";
    // contact server
    response = sendRecv(socketDescriptor, smtpMsg);
    resultCode = response.substr(0, 3);
    // stop if the server refuse delivery
    if (resultCode.compare("250") != 0) {
		// log error NO
		errno = resultCode;
		// print out error and return -1 if fail
        cerr <<response;
        close(socketDescriptor);
        freeaddrinfo(servInfo);
        return -1;
    }
    
    // set RECIPIENT TO
    smtpMsg = "RCPT TO: <" + envelope_to + ">";
    smtpMsg += "\r\n";
    // contact server
    response = sendRecv(socketDescriptor, smtpMsg);
    resultCode = response.substr(0, 3);
    // stop if the server refuse delivery
    if (resultCode.compare("250") != 0) {
		// log error NO
		errno = resultCode;
		// print out error and return -1 if fail
        cerr <<response;
        close(socketDescriptor);
        freeaddrinfo(servInfo);
        return -1;
    }
    
    // initialize DATA
    smtpMsg = "DATA";
    smtpMsg += "\r\n";
    // contact server
    response = sendRecv(socketDescriptor, smtpMsg);
    resultCode = response.substr(0, 3);
    // stop if the server refuse delivery
    if (resultCode.compare("354") != 0) {
		// log error NO
		errno = resultCode;
		// print out error and return -1 if fail
        cerr <<response;
        close(socketDescriptor);
        freeaddrinfo(servInfo);
        return -1;
    }
    
    // set mail content
    smtpMsg = mailContent + "\r\n.\r\n";;
    // contact server
    response = sendRecv(socketDescriptor, smtpMsg);
    resultCode = response.substr(0, 3);
    if (resultCode.compare("250") != 0) {
		// log error NO
		errno = resultCode;
		// print out error and return -1 if fail
        cerr <<response;
        close(socketDescriptor);
        freeaddrinfo(servInfo);
        return -1;
    }
    
    // successfully delivered inform
    cout <<"Your email was successfully delivered." <<endl <<endl;
    
    // Set QUIT message
    smtpMsg = "QUIT\r\n";
    // contact server
    sendRecv(socketDescriptor, smtpMsg);

    // done!
    close(socketDescriptor);
    freeaddrinfo(servInfo);
    
    // return 0 on success
    return 0;
}

/**
 * MailSender Send and Receive
 *	Process communication session with mailhost
 */
string MailSender::sendRecv(int sd, string &msg) {
    size_t byteSent;	// compare with message length
    char buff[999];		// catch response from server

    byteSent = send(sd, msg.c_str(), msg.length(), 0);
    // if byteSent mismatch
    if (byteSent != msg.length()) {
        cout << "Warning: Message was not fully transmitted.\n";
    }
    byteSent = recv(sd, buff, 999, 0);
    // just to ensure buff end with an endl character
    buff[byteSent] = '\0';
    return (string(buff));
}
