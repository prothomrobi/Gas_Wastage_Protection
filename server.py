from flask import Flask
from flask import request
from pymongo import MongoClient
app = Flask(__name__)

client = MongoClient('localhost:27017')
db=client.kitchen

 
@app.route('/')
def hello_world():
    return 'Hello, World!'

@app.route('/sensor',methods = ['POST', 'GET'])
def sensor():
    if request.method == 'POST':
        content = request.json 
        db.kitchenData.insert_one(content)
        return "done"
    else:
        return "error"

if __name__ == '__main__':
    app.run(host='0.0.0.0')
