/*
		***SONG_GEN***
	Song_gen is a simplistic packaging (No compression) application written for compacting files (Mainly Audio) for use with Project RX.
	The format, being *.song, this application stores audio files and other stuff needed for song charts in Project RX. This makes transferring them much easier.
	It also saves people the trouble of using directories to store files. However, it comes at the cost of having to decompress if you want to edit the chart data.

	Arguments:
		song_gen.exe <data_txt>
		data_txt - Text file containing the file names of the files to be stored.

	data_txt goes in the following format:

		[INFORMATION]
			name = <name>
			author = <name>
			year = <name>
			length = <name>
			genre = <name>
			album = <name>
			comment = <name>
		[AUDIO]
			file = <name>
			file = <name>
			file = <name>
			file = <name>
		[MISC]
			file = <name>
			file = <name>
			file = <name>

	You will likely use the GUI version to have this file generated for you.
*/

#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

using namespace std;

typedef unsigned char byte;

const int version = 2; //The current version of the file.

bool file_exists(string filename) {
	ifstream ifile(filename);
	return ifile;
}

string getFilename(string path) {
    int i;
    for (i = path.length() - 1; i >= 0; i--) {
        if (path[i] == '/' || path[i] == '\\')
            break;
    }
    return path.substr(i + 1,path.length() - i - 1);
}

string tostring(int integer) {
	string res;
	ostringstream convert;
	convert << integer;
	return convert.str();
}

byte bitstr_to_int(string bitstr) {
	byte integer = 0;
	for (int i = 0; i < bitstr.length(); i++)
		integer += pow((double)2, i) * (bitstr[i] == '1');
	return integer;
}

string to_hex(int number) {
	stringstream stream;
	stream << hex << number;
	return stream.str();
}

void Int_to_VLQ(int number, ofstream &fp) {
	//Count up how many bits this number will use.
	int _i;

	//Somewhat inefficient For Loop ftw
	for (_i = 0; number >= (2 << _i); _i++) {}

	//Add by 1.
	_i++;

	//Let's display those bits...
	string bitstr = "";
	for (int i = 0; i < _i; i++)
		bitstr += tostring((number & (1 << i)) != 0);

	//Initiate the byte count. This is necessary for many reasons.
	int bytecount = (_i / 7) + 1;
	byte bytecutoff = _i % 7;

	string* bytestr_of_VLQ = new string[bytecount];
	for (int i = 0; i < bytecount; i++) {
		for (int a = 7 * i; a <= 6 + (7 * i); a++) {
			if (a >= bitstr.length())
				bytestr_of_VLQ[i] += '0';
			else
				bytestr_of_VLQ[i] += bitstr[a];
		}
		bytestr_of_VLQ[i] += tostring(i != 0);
	}

	byte* bytes_of_VLQ = new byte[bytecount]; //Let's make some bytes... I think.
	for (int i = 0; i < bytecount; i++) {
		//Convert to integers... Well typedef bytes.
		bytes_of_VLQ[i] = bitstr_to_int(bytestr_of_VLQ[(bytecount - 1) - i]);
	}

	//Check for an unnecessary number... 0x80 can be the first number but it would mean absolutely nothing at all... Period.
	//On the plus side, it saves us an additional byte.
	if (bytes_of_VLQ[0] == 0x80) {
		for (int i = 0; i < bytecount - 1; i++) {
			//Shift them all down by 1.
			bytes_of_VLQ[i] = bytes_of_VLQ[i + 1];
		}
		bytecount -= 1;
	}

	cout << " (0x";
	for (int i = 0; i < bytecount; i++) {
		//cout << "0x" << to_hex((int)bytes_of_VLQ[i]) << " ";
		cout << to_hex(bytes_of_VLQ[i]);
		fp << bytes_of_VLQ[i];
	}
	cout << ")" << endl;
}

unsigned int getFileSize(string filename) {
	ifstream file( filename, ios::binary | ios::ate);
	return file.tellg();
}

void writeBinaryToPAK(ofstream &pak, string filename) {
	//Will open up file, read its contents, and then write to pak.
	ifstream fp;
	fp.open(filename, ios::binary);
	string filen = getFilename(filename);

	//Get Size
	fp.seekg(0, std::ios::end);
    unsigned int fileSize = fp.tellg();
    fp.seekg(0, std::ios::beg);

	std::vector<byte> file_contents(fileSize);
    fp.read((char*) &file_contents[0], fileSize);
	fp.close();

	cout << "    Writing file: " << filen;


	pak << (unsigned char)filen.length() << filen;
	Int_to_VLQ(file_contents.size(), pak);
	pak.write((char*) &file_contents[0],fileSize);
}

int main() {
	clock_t start, end;

	start = clock();

	ifstream data_txt;
	string filename = "data_txt.ini";
	string tmp_txt;

	string song_name = "", song_author = "", song_genre = "", song_album = "", song_length = "", song_comment = "", song_year = "";

	vector<string> AUDIO_FILES;
	vector<string> MISC_FILES;

	data_txt.open(filename);
	while (data_txt.eof() == false) { //Funny, my professor told me to never use "eof"... Ah well. :P
		getline(data_txt,tmp_txt);
		if (tmp_txt == "[INFORMATION]") {
			while (data_txt.eof() == false) {
				getline(data_txt,tmp_txt);
				istringstream file_check;
				file_check.clear();
				file_check.str(tmp_txt);
				string file_checker;
				file_check >> file_checker;
				if (file_checker == "name") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					song_name = tmp_txt.substr(epos,tmp_txt.length() - epos);
				}
				else
				if (file_checker == "author") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					song_author = tmp_txt.substr(epos,tmp_txt.length() - epos);
				}
				else
				if (file_checker == "year") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					song_year = tmp_txt.substr(epos,tmp_txt.length() - epos);
				}
				else
				if (file_checker == "length") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					song_length = tmp_txt.substr(epos,tmp_txt.length() - epos);
				}
				else
				if (file_checker == "genre") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					song_genre = tmp_txt.substr(epos,tmp_txt.length() - epos);
				}
				else
				if (file_checker == "album") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					song_album = tmp_txt.substr(epos,tmp_txt.length() - epos);
				}
				else
				if (file_checker == "comment") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					song_comment = tmp_txt.substr(epos,tmp_txt.length() - epos);
				}
				else
					break;
			}
		}
		if (tmp_txt == "[AUDIO]") {
			while (data_txt.eof() == false) {
				getline(data_txt,tmp_txt);
				istringstream file_check;
				file_check.clear();
				file_check.str(tmp_txt);
				string file_checker;
				if (file_check >> file_checker && file_checker == "file") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					AUDIO_FILES.push_back(tmp_txt.substr(epos,tmp_txt.length() - epos));
					//cout << file_exists(AUDIO_FILES[AUDIO_FILES.size() - 1]) << endl;
				}
				else
					break;
			}
		}
		if (tmp_txt == "[MISC]") {
			while (data_txt.eof() == false) {
				getline(data_txt,tmp_txt);
				istringstream file_check;
				file_check.clear();
				file_check.str(tmp_txt);
				string file_checker;
				if (file_check >> file_checker && file_checker == "file") {
					unsigned int epos = tmp_txt.find("= ") + 2;
					MISC_FILES.push_back(tmp_txt.substr(epos,tmp_txt.length() - epos));
				}
				else
					break;
			}
		}
		break;
	}
	data_txt.close();

	cout << "Beginning writing file..." << endl << endl;
	ofstream pak;
	pak.open("pak.sxen", ios::binary);
	pak << "DERPG_SONGPAK" << (byte)(version / 256) << (byte)(version % 256);

	//Write Song Information
	pak << (byte) song_name.length() << song_name;
	pak << (byte) song_author.length() << song_author;
	pak << (byte) song_year.length() << song_year;
	pak << (byte) song_length.length() << song_length;
	pak << (byte) song_genre.length() << song_genre;
	pak << (byte) song_album.length() << song_album;
	pak << (byte) song_comment.length() << song_comment;

	//Begin header for Audio Files
	pak << (byte) 0xFF;
	pak << (byte) 5;
	pak << "AUDIO";

	cout << "Writing Audio Files" << endl;
	for (int i = 0; i < AUDIO_FILES.size(); i++) {
		pak << (byte) 0x00;
		writeBinaryToPAK(pak, AUDIO_FILES[i]);
	}
	cout << endl;

	//Header for Misc Files
	pak << (byte)0xFF;
	pak << (byte)4;
	pak << "MISC";
	cout << "Writing Misc Files" << endl;
	for (int i = 0; i < MISC_FILES.size(); i++) {
		pak << (byte) 0x00;
		writeBinaryToPAK(pak, MISC_FILES[i]);
	}
	
	end = clock();
	pak.close();
	cout << endl << "File Writing complete (" << (double)(end - start)/CLOCKS_PER_SEC << "s" << ")." << endl;

	getchar();
	return true;
}