/**
 * Copyright 2010 by Long Nguyen
 * Please included license in COPYING
 * 	or visit http://opensource.org/licenses/mit-license.php
 */

/**
 * Main.cc
 * Mail Program Entry Point
 */

#include <iostream>
#include <fstream>
#include <string>
#include "MailSender.hh"

using namespace std;

/**
 * This function is called if user didn't input enough arguments
 */
void missingOperand() {
    cout << "email: missing operand\n"
         << "Try 'email help' for more information.\n";
}

int main(int argc, char **argv) {
	/*
	string filename = "message.txt";
	MailSender *sender = new MailSender(filename);
	cout <<"This is a test\nAnother test\n";
	return 0;
	*/
	
	if (argc != 2) {
        missingOperand();
    } else {
        ifstream fout;
		string commandHelp(MailSender::COMMAND_HELP);
		string commandInstall(MailSender::COMMAND_INSTALL);
		string commandReadme(MailSender::COMMAND_README);
		string commandLicense(MailSender::COMMAND_LICENSE);
		// if special command is detected
		if (commandHelp.compare(argv[1]) == 0 || commandInstall.compare(argv[1]) == 0 || commandReadme.compare(argv[1]) == 0 || commandLicense.compare(argv[1]) == 0)
		{
			// print according file
				if (commandHelp.compare(argv[1]) == 0)
					fout.open("HELP");
				else if (commandInstall.compare(argv[1]) == 0)
					fout.open("INSTALL");
				else if (commandReadme.compare(argv[1]) == 0)
					fout.open("README");
				else
					fout.open("COPYING");
			string line;
			while (getline(fout, line) != 0) {
				cout << line << endl;
			}
			fout.close();
		} else {
			// create email instance with the given filename
			string filename(argv[1]);
			MailSender sender(filename);
			sender.process();
		}
    }
    
    return 0;
}
