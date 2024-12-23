#include <Audio.h>

#include "cloud.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include "params.h"

const char* myAccessToken = "ya29.c.c0ASRK0GbyFB_cyPzvhcqCrL34CCQvje3y1NrECaqqtCU00qoNOcDA6Ovhm4aSNJLA21WwcKigFdmDiCl8KYSIqg25RkaDO4ts981h8fRW3camf0ED1B63mDThad-m1OTVMxKKpkEveAUxRse5-kz7u-ieyuC2jv2fIEJISjVgTvbJ3FjFnvzZZCCpaNctkgqQlhxFS-INt22z-F8dRJQ8sZSHDCqLem27YP3k1kwpFhx5hA7041-3PxTbRfckJW6zQpMqWiVNv9Lkzt3g8gk_Hb9z_E1-Va0Lz28b6xiUjl9VXyZToPjAUF8MnDnT5Oo9SEU1KtoIjIvs-wJLUPa1LicS2PYId88ewiZPO0CfOss59gZHUELhp9YT384AY3093hj3ZciOZg1Syy2cdbz00ytUBhioVkRofrprtb4YYyXW4xx0sYIoU7Xh2xX0Idl5sfQybVzRfYxrVhXQakqIMwlW41axyk7X2U8_d0Onjqq5OdnxS6Mzjqk5M-wOYqJlfe13Rlm_k6etbib9x0SgW3cFu13bpVbIrziuxyU1V-XFdisd26mkvpuSo1Qud1_ou4_evSSZMWsm_v_Wc5aYmR2pYRVk81QzO2n0xISF0FouwXV1ptnjSso2x17tjz1agbzWz70Fqm-yw8u4wk6_v6p3xiieU5Xd7UrMuz_Uc79w6zew6Re_X_eaMnQOzlWJhVcMd8nrrJqIVi4qdzorticxQ5JMcBOubnpw_qz-dj4FirnsX4bhecrV02uaIgb5YvcurznFegjlVRR32g4c39-cS6ufkeuaazs55S_79oFMVWltafSh9z20l5u1XxnWvdamxy8kiIQhg72yQxfe1a7d6l-ksjRs3zR6ai4s3tuyyqFbu-yJY5706YzMZic2Qse_i6e7sp-1J7f354XQ-kYQrfn9RmjighOW2BS3e7X2it42MRmnS03hBW1vev7BeOIgze0hnZydVsBSp8vtY5hwI9YaY_e25Q_lw-zVUIQj3zFviUurs10";

// STT Config
const char* bucketName = "beuzeboc-turtle";
const char* audioContent = "gs://beuzeboc-turtle/audio.wav";
const char* gcAdd = "https://storage.googleapis.com/upload/storage/v1/b/beuzeboc-turtle/o?uploadType=media&name=audio.wav";

// Gemini API Config
const char* apiURL = "https://europe-west1-aiplatform.googleapis.com/v1/projects/lithe-bonito-212721/locations/europe-west1/publishers/google/models/gemini-1.5-flash-002:streamGenerateContent?alt=sse";


void uploadFile() {
  Serial.println("Opening file...");
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file");
    file.close();
    return;
  }
  Serial.println("File opened successfully");

  HTTPClient _http;

  // String url = "https://storage.googleapis.com/upload/storage/v1/b/" + String(_bucketName) + "/o?uploadType=media&name=audio.wav";
  Serial.print("Connecting to URL: ");
  Serial.println(gcAdd);
  if(!_http.begin(gcAdd)){
    Serial.println("Failed to begin http request with gcAdd");
    file.close();
    return;
  }
  _http.addHeader("Authorization", "Bearer " + String(myAccessToken));
  _http.addHeader("Content-Type", "application/octet-stream");

  Serial.println("Sending POST request...");
  Serial.println(file.size());
  int httpCode = _http.sendRequest("POST", &file, file.size());
  Serial.println("POST request sent");

  if (httpCode > 0) {
    String response = _http.getString();
    // Serial.println("Response: " + response);
  } else {
    Serial.println("Error on file upload, HTTP code: " + String(httpCode));
  }

  file.close();
  _http.end();
  Serial.println("File closed and HTTP connection ended");
}

const String speechRequestData = "{\"config\": {\"encoding\":\"LINEAR16\",\"languageCode\":\"en-US\",\"enableWordTimeOffsets\":false},\"audio\":{\"uri\":\"gs://beuzeboc-turtle/audio.wav\"}}";
String speechToText() {
  Serial.println("Speech to text start.");
  HTTPClient _http;

  _http.begin("https://speech.googleapis.com/v1/speech:recognize");
  _http.addHeader("Content-Type", "application/json");
  _http.addHeader("Authorization", "Bearer " + String(myAccessToken));
  _http.addHeader("User-Agent", "PostmanRuntime/7.40.0");
  _http.addHeader("Accept", "*/*");
  _http.addHeader("Accept-Encoding", "gzip, deflate, br");
  _http.addHeader("Connection", "keep-alive");
  _http.addHeader("x-goog-user-project", "lithe-bonito-212721");

  Serial.println("Speech request.");
  Serial.println("Speech request post start.");
  int httpResponseCode = _http.POST(speechRequestData);

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode == 200) {
    String response = _http.getString();
    // Serial.println("Response: ");
    // Serial.println(response);

    int transcriptIndexStart = response.indexOf("\"transcript\": \"") + 15;
    int transcriptIndexEnd = response.indexOf("\"", transcriptIndexStart);

    if (transcriptIndexStart > 14 && transcriptIndexEnd > transcriptIndexStart) {
      String transcript = response.substring(transcriptIndexStart, transcriptIndexEnd);
      Serial.print("Transcript: ");
      Serial.println(transcript);
      return transcript;
    } else {
      Serial.println("Transcript not found.");
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  _http.end();
  return "";
}

String generateJsonString(String text) {
  return "{"
         "\"contents\": ["
         "{"
         "\"role\": \"USER\","
         "\"parts\": { \"text\": \"Now, you are a small female robo turtle, your name is Amy. You will be a helpful AI assistant. Your LLM api is connected to STT and TTS models so you are able to hear the user.\" }"
         "},"
         "{"
         "\"role\": \"MODEL\","
         "\"parts\": { \"text\": \"OK, I know.\" }"
         "},"
         "{"
         "\"role\": \"USER\","
         "\"parts\": { \"text\": \""
         + text + "\" }"
                  "}"
                  "]"
                  ",\"safety_settings\": {"
                  "\"category\": \"HARM_CATEGORY_SEXUALLY_EXPLICIT\","
                  "\"threshold\": \"BLOCK_LOW_AND_ABOVE\""
                  "},"
                  "\"generation_config\": {"
                  "\"temperature\": 0.2,"
                  "\"topP\": 0.8,"
                  "\"topK\": 40,"
                  "\"maxOutputTokens\": 20"
                  "}"
                  "}";
}


String llm_response(String transcript) {
  HTTPClient _http;

  Serial.println("Start sending HTTP POST request...");
  _http.begin(apiURL);
  _http.addHeader("Content-Type", "application/json");
  _http.addHeader("Authorization", "Bearer " + String(myAccessToken));

  // Constrct JSON data
  // String jsonData = "{\"contents\": {\"role\": \"user\", \"parts\": [{\"text\": \"" + transcript + "\"}]}}";
  String jsonData = generateJsonString(transcript);
  // Serial.print("Sent JSON data: ");
  // Serial.println(jsonData);

  int httpResponseCode = _http.POST(jsonData);

  String geminiResponseText = "";
  if (httpResponseCode > 0) {
    String response = _http.getString();
    // Serial.print("HTTP repond code: ");
    // Serial.println(httpResponseCode);
    // Serial.print("Responded data: ");
    // Serial.println(response);

    int startIndex = 0;
    while (true) {
      startIndex = response.indexOf("\"text\": \"", startIndex);
      if (startIndex == -1) break;
      startIndex += 9;
      int endIndex = response.indexOf("\"", startIndex);
      if (endIndex == -1) break;
      geminiResponseText += response.substring(startIndex, endIndex) + " ";
      startIndex = endIndex + 1;
    }

    Serial.print("Combined text: ");
    Serial.println(geminiResponseText);
  } else {
    Serial.print("Request failed, HTTP respond code: ");
    Serial.println(httpResponseCode);
  }

  _http.end();
  return geminiResponseText;
}
