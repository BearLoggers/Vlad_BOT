#include <iostream>
#include <SFML/Network.hpp>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>

#include <locale>
#include <codecvt>
#include <io.h>
#include <fcntl.h>


bool file_exists(const char* fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}


int main()
{
	setlocale(LC_ALL, "rus");
	sf::TcpListener listener;
	sf::SocketSelector selector;

	std::ofstream file;

	std::vector<sf::TcpSocket*> sockets;

	int port;
	std::cout << "Введите порт: ";
	std::cin >> port;	// Указываем порт, который открываем для работы

	if (listener.listen(port) != sf::Socket::Done)
	{
		std::cout << "Err.";
		exit(1);
	}

	std::thread nodeThread([]() // Создаем поток, в котором запускаем node
	{
		system("cd ../../JS_BOT && node ./server.js");
	});

	nodeThread.detach();

	std::cout << "Сервер слушает " << port << '\n';
	selector.add(listener);

	while (true) // сервер работает в бесконечном цикле
	{
		if (selector.wait())
		{
			std::cout << "сервер дождался" << std::endl;
			if (selector.isReady(listener))
			{
				sf::TcpSocket* socket = new sf::TcpSocket;

				if (listener.accept(*socket) != sf::Socket::Done)
				{
					// Ошибка при подключении
					std::cout << "Что-то пошло не так :(" << std::endl;
				}
				else
				{
					// Подключение успешно, добавляем к прослушке
					std::cout << "Получил нового клиента" << std::endl;
					selector.add(*socket);
					sockets.push_back(socket);
				}
			}
			// Проходимся по добавленым сокетам и определяем, кто из них сработал
			else
			{
				for (int i = 0; i < sockets.size(); i++)
				{
					if (selector.isReady(*sockets[i]))
					{
						sf::Packet packet;
						sf::Socket::Status status = sockets[i]->receive(packet); // Принимаем входящий пакет от сокета
						if (status == sf::Socket::Disconnected)
						{
							selector.remove(*sockets[i]);
							delete sockets[i];
							sockets.erase(sockets.begin() + i);
							std::cout << "Клиент удачно отключился.\n";
							continue;
						}
						else if (status == sf::Socket::Done)
						{
							std::cout << "Принял немного данных: " << std::endl;
							short choice;	// Выбор метода воспроизведения от пользователя.
							packet >> choice;

							std::cout << "Выбор: " << choice << std::endl;

							std::string data, temp, url;

							switch (choice)
							{
							// Поиск по VK
							case 1:
							{
								packet >> data;  // строка поиска по аудио в вк
								std::cout << "Клиент хочет искать по ВК: " << data << std::endl;

								file.open("../../JS_BOT/vkquery.txt");
								file << data;
								file.close();

								
								std::cout << "Ожидаю ответа от Node.JS...\n";
								while (!file_exists("vksearch.status"));		// Ждём результата от Node.JS
								std::cout << "Дождался!\n";
								std::ifstream status("vksearch.status");
								std::string string;
								std::getline(status, string, '\n');
								if (string == "success")
								{
									std::getline(status, string, '\n');		// Название и автор

									packet.clear();
									packet << "success" << string;
									sockets[i]->send(packet);
								}
								else
								{
									packet.clear();
									packet << "failed" << string;
									sockets[i]->send(packet);
								}

								status.close();
								system("del vksearch.status");

								break;
							}

							// Произвольная ссылка
							case 2:
								packet >> data;		// ссылка
								std::cout << "Клиент хочет воспроизводить по ссылке: " << data << std::endl;
								file.open("../../JS_BOT/link.txt");
								file << data;
								file.close();
								break;

							default:
								std::cout << "У пользователя обнаружен модифицированный клиент.\n";
								break;
							}
						}
						else
						{
							std::cout << "Ошибка при приёме пакета!\n";
						}
					}
				}
			}
		}
		else
		{
			std::cout << "Ошибка во время ожидания.\n";
		}
	}
}
