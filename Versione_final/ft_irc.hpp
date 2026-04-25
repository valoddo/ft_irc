/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cacorrea <cacorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/25 16:58:30 by cacorrea          #+#    #+#             */
/*   Updated: 2026/04/25 17:07:19 by cacorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_IRC_HPP
# define FT_IRC_HPP

# define SERVER_NAME "Grapita"

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <sstream>
# include <cstring>
# include <cstdlib>
# include <cerrno>
# include <ctime>
# include <unistd.h>
# include <fcntl.h>
# include <poll.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>

class Client;
class Channel;
class Server;

#endif
