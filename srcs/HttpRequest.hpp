/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shechong <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/16 14:41:49 by shechong          #+#    #+#             */
/*   Updated: 2025/05/16 14:41:50 by shechong         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../includes/Webserv.hpp"
#include "Utils.hpp"

class HttpRequest {
private:
    string     method;
    string     body;
    stringDict headerParameters;
    string     rawRequest;
    string     raw;
    string     queryString;
    string     absolutePath;
    string     _path;
    string     _uid;
    string     hostNumber;

public:
    

    /* CONSTRUCTOR/DESTRUCTOR */
    
    HttpRequest();
    ~HttpRequest();

    void        appendFormBlock(stringDict formBlock);
    stringDict *getFormBlock(int index);
    void        setBody(string body);
    string      &getBody();


    /* GETTER/SETTERS */

    void   setMethod(string method);
    string getMethod() const;
    
    void   setQueryString(string queryString);
    string getQueryString() const;
    
    void   setPath(string path);
    string getPath() const;
    void   setHost(string host);
    string getHost();

    void   headerSet(string key, string value);
    string headerGet(string key);

    string const &getUid() const;

    void   setRawRequest(string rawRequest);
    string getRawRequest() const;


    string preview();
    
};

#endif
