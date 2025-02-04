// HttpRequest *MockRequest() {
//     HttpRequest *request1 = new HttpRequest(GET, "fusion_web/about.html", "HTTP/1.1");
    
//     string requestContentRaw = readFileContent("testRequest");
//     std::cout << requestContentRaw << std::endl;

//     std::istringstream requestStream(requestContentRaw);
//     string line;
//     while (std::getline(requestStream, line)) {
//         std::vector<string> parts = splitByFirstInstance(line, ':');
//         if (parts.size() == 2)
//             request1->setKey(parts[0], parts[1].substr(1));
//     }

//     // iterate for each key
//     const std::map<string, string>& params = request1->getRequestParameters();
//     if (!params.empty()) {
//         for (auto it = params.begin(); it != params.end(); ++it)
//             std::cout << it->first << " : " << it->second << std::endl;
//     } 
//     else
//         std::cerr << "Request parameters are empty." << std::endl;

//     string fullURL = request1->getParam("Referer");
//     string path = splitByNthInstance(fullURL, '/', 3)[1];
//     path = path.substr(0, path.size() - 1);
//     request1->setKey("path", "profile.html");
//     std::cout << "path: " << request1->getParam("path") << std::endl;

//     return request1;
// }
