from flask import Flask, jsonify, request
from flask_cors import CORS
import requests



app = Flask(__name__)
CORS(app) #If you want access of endpoint then origin = '*'

API_KEY = "INSERT-HERE"
API_ASSISTANT = "INSERT-HERE"
##############################################################################################
@app.route('/createThread', methods=['POST'])
def createThread():
    url = "https://api.openai.com/v1/threads"
    headersNew = {
                "Content-Type": "application/json",
                "Authorization": f"Bearer {API_KEY}",
                "OpenAI-Beta": "assistants=v2"
            }
    response = requests.post( url, headers=headersNew)
    if response.status_code == 200:
        responseContent = response.json()
        return jsonify({"thread_id": response.json()["id"]}), 200
    else:
        return 403

@app.route("/getResponse", methods=['POST'])
def askQuestion():
    threadID = request.json["threadID"]
    message = request.json["msg"]
    print(threadID, message)
    url1 = f"https://api.openai.com/v1/threads/{threadID}/messages"
    url2 = f"https://api.openai.com/v1/threads/{threadID}/runs"
    headersNew = {
                "Content-Type": "application/json",
                "Authorization": f"Bearer {API_KEY}",
                "OpenAI-Beta": "assistants=v2"
            }
    bodyNew1 = {
                "role": "user",
                "content": f"In 2 to 4 sentences, {message}"
            }
    bodyNew2 = {
                "assistant_id" : f"{API_ASSISTANT}",
                "stream" : True
            }
    response1 = requests.post(url1, headers=headersNew, json=bodyNew1)#create a new message
    #print(response1.content)
    response2 = requests.post(url2, headers=headersNew, json=bodyNew2)#create a new run
    #print(response2.content)
    response3 = requests.get(url1, headers=headersNew)#Looks at message
    #print(response3.content)
    if response3.status_code == 200:
        return jsonify({"message": response3.json()['data'][0]['content'][0]['text']['value']}), 200
        #return jsonify({"message": response3.json()})
    else:
        return 403
    #return "rat"

