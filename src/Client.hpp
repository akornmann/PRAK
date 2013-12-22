#ifndef CLIENT_H
#define CLIENT_H

//Usefull includes
#include "socket.hpp"

//Data structures
#include "addr_map.hpp"
#include <vector>

//Class
#include "AddrStorage.hpp"
#include "Converter.hpp"
#include "Datagram.hpp"
#include "Exception.hpp"
#include "File.hpp"
#include "State.hpp"
#include "Counter.hpp"

using namespace std;

enum Protocol
{
	CONNECTRA,
	DISCONNECTRA,
	DOWNLOAD,
	UPLOAD,
};


typedef vector<Exception> exc;

class Client
{
 public :
	Client(string &config);
	~Client();

	int sock(const AddrStorage &addr);

	//Envoi/reception de datagrammes
	void send_to(const Datagram &dg, const AddrStorage &addr);
	bool receive(Datagram &dg, AddrStorage *addr);
	bool receive_from(Datagram &dg, const AddrStorage &addr);

	//Protocole de base
	void connect_req(const AddrStorage &addr);
	void disconnect_req(const AddrStorage &addr);
	void get_file(const string &file, const AddrStorage &addr);
	void send_file(const string &file, const string &title, const AddrStorage &addr);

	//Surcouche client
	void synchronize(AddrStorage *addr);
	void get_file(string file);
	void send_file(string file, string title);

 private :
	int _sock_4;
	int _sock_6;

	addr_map _server_map;
};

#endif
