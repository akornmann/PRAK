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
	bool connect(const AddrStorage &addr);
	bool synchronize(AddrStorage *addr);

	//Envoi/reception de datagrammes
	bool send_to(const Datagram &dg, const AddrStorage &addr);
	bool receive(Datagram &dg, AddrStorage *addr);
	bool receive_from(Datagram &dg, const AddrStorage &addr);

	//Protocole de base
	bool connect_req(const AddrStorage &addr);
	bool disconnect_req(const AddrStorage &addr);
	bool get_file(const string &file, const AddrStorage &addr);
	bool send_file(const string &file, const string &title, const AddrStorage &addr);

	//Surcouche client
	bool get_file(string file);
	bool send_file(string file, string title);

	//Gestion erreurs
	exc error();
	
 private :
	int _sock_4;
	int _sock_6;

	addr_map _server_map;

	exc _exc;
};

#endif
