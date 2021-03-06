﻿// это клиент

#include <iostream>
#include <SFML/Network.hpp>
#include <string>
#include <thread>
#include <chrono>

std::string ipaddress;
unsigned short port;

bool isDisconnected(sf::Socket::Status status, sf::TcpSocket& socket) {
	if (status == sf::Socket::Status::Disconnected) {
		std::cout << "Потеряно соединение, пробую ещё раз";
		while (socket.connect(ipaddress, port) != sf::Socket::Status::Done) {
			std::cout << ".";
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		std::cout << "Соединение восстановлено\n";
		return true;
	}

	return false;
}

void sendAndCheck(sf::TcpSocket& socket, sf::Packet& packet) {
	if (isDisconnected(socket.send(packet), socket)) {
		// Даём серверу инициализоваться
		std::this_thread::sleep_for(std::chrono::seconds(5));
		socket.send(packet);
	}
}

int main()
{
	setlocale(LC_ALL, "rus");
	short choice = 0;
	std::string string, temp;

	std::cout << "Введите IP (Enter для ip.valler.net): ";
	std::getline(std::cin, ipaddress, '\n');
	if (ipaddress.empty()) ipaddress = "ip.valler.net";

	std::cout << "Введите port (Enter для 25565): ";
	std::getline(std::cin, temp, '\n');
	if (temp.empty()) port = 25565;
	else port = std::stoi(temp);

	sf::TcpSocket socket;

	std::cout << "Попытка подключения";
	while (socket.connect(ipaddress, port) != sf::Socket::Done)
	{
		std::cout << '.';
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << " успешна!\n";

	while (true)
	{
		std::cout << "Чего пожелаете?\n 1 - Поиск по VK.\n 2 - Проигрывание с сырой ссылки.\n";
		std::cin >> choice;
		sf::Packet packet;

		switch (choice)
		{
		case 1:
			std::cout << "Введите название композиции: ";
			std::cin.ignore();
			std::getline(std::cin, string);
			if (string.empty())
				continue;
			packet << choice << string;
			sendAndCheck(socket, packet);
			
			std::cout << "Ожидаю ответа от сервера... ";
			socket.receive(packet);

			packet >> string;
			std::cout << string <<'\n';

			if (string == "success")
			{
				packet >> string;
				std::cout << "Проигрывается: '" << string << "'\n";
			}
			
			break;


		case 2:
			std::cout << "Введите ссылку: ";
			std::cin >> string;
			if (string.empty())
				continue;
			packet << choice << string;
			sendAndCheck(socket, packet);

			break;

		default:
			std::cout << "Я не понимаю, чего вы хотите.\n";
			break;
		}

	}
}