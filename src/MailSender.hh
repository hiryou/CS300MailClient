/**
 * Copyright 2010 by Long Nguyen
 * Please included license in COPYING
 * 	or visit http://opensource.org/licenses/mit-license.php
 */

/**
 * MailSender.hh
 * Mail Sender Class Declaration
 */

#ifndef MAILSENDER

#define MAILSENDER

using namespace std;

class MailSender
{

public:
    public:
    /**
	 * Set up the class for sending the given file as email.
	 * The file should contain an RFC-822 formatted email
	 * message, whose correctness will be assumed by this object.
	 */
    MailSender(string &filename);
    
    /**
     * list of available commands within the email package
     */
    static const std::string COMMAND_HELP;
    static const std::string COMMAND_INSTALL;
    static const std::string COMMAND_README;
    static const std::string COMMAND_LICENSE;
    
    /**
     * Default mailhost and connection port
     */
    static const std::string DEFAULT_MAILHOST;
    static const std::string DEFAULT_PORT;
    
    int process();
    
    
    /**
	 * Actually send the email to the given host
	 * with the given source and recipient.  Returns
	 * 0 on success, and -1 on failure.  In the
	 * case of failure, the global variable errno
	 * will be set to indicate a failure code.
	 */
    virtual int deliver(string &host_to,
			string &envelope_from,
			string &envelope_to);
			
	private:
	int status;				// status of the process, 0 = success, 1 = fail
	string errno;			// indicate error
	string mailhost;		// mailhost used to relay mail, default is "mailhost.cs.pdx.edu"
	string port;			// what port to connect to mailhost
	string emailFile;		// path to the file containing the email content
	string mailContent;		// holds mail content from From:, To:... to "."
	
	string getMailElement(string &msg, string target);
	string trim(string &str);
	string sendRecv(int sd, string &msg);
};

#endif
