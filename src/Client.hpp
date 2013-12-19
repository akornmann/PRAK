/*
 * Alexandre Kornmann
 * Projet reseau
 * Master 2 CSMI
 * Classe Client : gestion de la connexion avec le serveur
 */

#include "socket.hpp"
#include "addr_map.hpp"

#include "AddrStorage.hpp"
#include "File.hpp"
#include "Datagram.hpp"
#include "Exception.hpp"
#include "State.hpp"
#include <vector>

using namespace std;

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
	bool toctoc(Datagram &dg, const AddrStorage &addr);
	bool get_file(const string &file, const AddrStorage &addr);
	bool send_file(const string &file, const AddrStorage &addr);

	//Surcouche client
	bool get_file(string file);
	bool send_file(string file);

	//Gestion erreurs
	exc error();
	
 private :
	int _sock_4;
	int _sock_6;

	addr_map _server_map;

	exc _exc;
};
