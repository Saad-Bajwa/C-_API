#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output)
{
  size_t totalSize = size * nmemb;
  output->append((char *)contents, totalSize);
  return totalSize;
}

string loginAndGetToken(const string &username, const string &password)
{
  CURL *curl;
  CURLcode res;
  string readBuffer;
  string token = "";

  curl = curl_easy_init();
  if (curl)
  {
    json requestBody = {
        {"email", username},
        {"password", password}};

    string jsonString = requestBody.dump();

    curl_easy_setopt(curl, CURLOPT_URL, "https://testpms.digitalm.cloud/api/login");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonString.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Expect: ");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    cout << "Result from api : " << res << endl;

    if (res != CURLE_OK)
    {
      cerr << "Login request failed: " << curl_easy_strerror(res) << endl;
    }
    else
    {
      cout << "Raw API Response: " << readBuffer << endl; // Print full response
      try
      {
        json responseJson = json::parse(readBuffer);
        if (responseJson.contains("data"))
        {
          token = responseJson["data"]["token"];
          cout << "Login successful. Token: " << token << endl;
        }
        else
        {
          cerr << "Login failed. No token in response." << endl;
        }
      }
      catch (json::parse_error &e)
      {
        cerr << "Failed to parse JSON response: " << e.what() << endl;
      }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
  return token;
}

void sendSaleRequest(const string &token)
{
  CURL *curl;
  CURLcode res;
  string readBuffer;

  curl = curl_easy_init();
  if (curl)
  {
    json saleJson = {
        {"id", "123"},
        {"vrdate", "2025-03-13"},
        {"vrtime", "14:30:00"},
        {"employee_id", "2"},
        {"total_amount", "5000"},
        {"bill_discount", "20.005"},
        {"nozzle_id", "7"},
        {"ledger_id", ""}};

    json saleDetailItem = {
        {"nozzle_id", "7"},
        {"product_id", "101"},
        {"quantity", "10"},
        {"rate", "50"},
        {"amount", "500"}};

    json saleDetailArray = json::array();
    saleDetailArray.push_back(saleDetailItem);

    // Convert to string before sending
    json finalPayload = {
        {"sale", saleJson.dump()},             // Convert to string
        {"saleDetail", saleDetailArray.dump()} // Convert to string
    };

    string jsonString = finalPayload.dump();
    cout << "JSON Payload to be sent: " << jsonString << endl;

    curl_easy_setopt(curl, CURLOPT_URL, "https://testpms.digitalm.cloud/api/liveNozzleSave");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    string authHeader = "Authorization: Bearer " + token;
    headers = curl_slist_append(headers, authHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
    }

    cout << "Response from API:\n"
         << readBuffer << endl;
    cout << "Stringified Response: " << json::parse(readBuffer).dump(4) << endl;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }
}

int main()
{
  cout << "Starting the program..." << endl;
  string username = "mmg@digitalsofts.com";
  string password = "150187%^#$*)&_)";

  string token = loginAndGetToken(username, password);
  if (!token.empty())
  {
    sendSaleRequest(token);
  }
  else
  {
    cout << "Failed to authenticate." << endl;
  }

  return 0;
}
