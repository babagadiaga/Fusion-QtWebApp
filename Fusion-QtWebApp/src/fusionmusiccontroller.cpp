#include "fusionmusiccontroller.h"
#include "requestmapper.h"
#include "template.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QtEndian>

FusionMusicController::FusionMusicController (QObject* parent) : HttpRequestHandler(parent)
{
    // Empty
}



void FusionMusicController::service(HttpRequest &request, HttpResponse &response)
{


    if (request.hasParameter("deviceName", request.getParameter("deviceName")))
    {
        QByteArray deviceName = request.getParameter("deviceName");
        QByteArray com = "fapiSetDeviceName";

        byteMerger(com, deviceName);
    }


    if (request.hasParameter("songState", request.getParameter("songState")))
    {
       QByteArray songState = request.getParameter("songState");
       QByteArray com = "fapiSetMedia";
    }




    if (request.hasParameter("sourceType", request.getParameter("sourceType")))
    {
        QByteArray sourceType = request.getParameter("sourceType");
        QByteArray com = "fapiSetSource";
    }



    if (request.hasParameter("powerState", request.getParameter("powerState")))
    {
        QByteArray powerState = request.getParameter("powerState");
        QByteArray com = "fapiSetPowerState";
        byteMerger(com, powerState);
    }







    // Code for webpage generation here

    if (true) {

    // Simple HTML5 webpage to be sent to the client, for Fusion_QtWebApp Music UI Application

    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

    response.write("<html>");

    response.write("<head>");

    response.write("<link type='text/css'  rel='stylesheet' href='/fusionMusic/fusion.css'/>");
    response.write("<script type='text/javascript' src='/fusionMusic/fusion.js'></script>");

     response.write("</head>");

     response.write("<body>");



     response.write("<div>");
       response.write("<button class='dName centerElements' type='button' id='moreInfoButton'>More Information</button>");

       response.write("<h1 class='dName centerElements' id='deviceName'>Device Name:<h1>");


         response.write("<input class='dName centerElements' type='text' id='setDeviceName'>");


       response.write("<h2 class='dName centerElements' id='name'>Cyrus<h2>");
       response.write("<button class='centerElements' type='button' id='changeButton' onclick='changeName()'>CHANGE</button>");
       response.write("<p class='centerElements' id='warningLabel'>Warning! Name cannot be longer than 10 characters.<p>");

       response.write("<button class='centerElements' type='button' id='playPauseButton' onclick='playPause()'>Play</button>");

       response.write("<select class='comboBox centerElements' id='choiceBox' onchange='sourceChange()'>");
         response.write("<option value='AM'>AM</option>");
         response.write("<option value='FM'>FM</option>");
         response.write("<option value='AUX'>AUX</option>");
         response.write("<option value='Bluetooth'>Bluetooth</option>");
       response.write("</select>");

       response.write("<h2 class='dName centerElements' id='sourceTitle'>Source Type:<h2>");

       response.write("<label class='switch centerElements' id='powerSwitch' >");
         response.write("<input type='checkbox' id='switch' onclick='powerChange();'/>");
         response.write("<span class='slider round'></span>");
       response.write("</label>");

       response.write("<h3 class='centerElements' id='onLabel'>OFF</h3>");

       response.write("<h3 class='dName centerElements' id='foot1'>Powered by </h3><h2 class='dName centerElements' id='foot2' style='color: #03c46a'>Fusion Entertainment</h2>");

     response.write("</div>");



     response.write("</body>");

     response.write("</html>", true);
    }

}




/**
 * @brief function that maps fapiCommand to corresponding little Endian encoding of command bytes ID
 * @param command
 * @return two-bytes QByteArray, containing formatting of command
 */
QByteArray FusionMusicController::commandEncode(QByteArray &command)
{
    QByteArray output(2, 0);

    if (command == "fapiGetState") {
        output[0] = 0x01;
        output[1] = 0x00;
    } else if (command == "fapiSetDeviceName") {
        output[0] = 0x1d;
        output[1] = 0x00;
    } else if (command == "fapiSetPowerState") {
        output[0] = 0x1c;
        output[1] = 0x00;
    } else if (command == "fapiSource") {
        output[0] = 0x02;
        output[1] = 0x80;
    } else if (command == "fapiSetSource") {
        output[0] = 0x02;
        output[1] = 0x00;
    } else if (command == "fapiSetMedia") {
        output[0] = 0x03;
        output[1] = 0x00;
    } else {
        output = "INVALID COMMAND";
    }

    return output;
}



/**
 * @brief command that uses all the other individual formatting functions, to concatenate and print out the QByteArray, for the corresponding interaction with the webpage
 * @param command - the fapi Command, which determines how the QByteArray is formatted/written, and how the message variable is read
 * @param message - the actual content of the message itself, containing flags (e.g on/off, deviceName etc)
 */
void FusionMusicController::byteMerger(QByteArray &command, QByteArray &message)
{

    QByteArray outputPacket;

    // Set header byte
    outputPacket[0] = 0xff;


    // Set message length bytes
    QByteArray messageLength;
    int messageSize = message.size();



    qDebug() << "THE SIZE OF THE MESSAGE FOR THE: " << command << "COMMAND, IS: " << messageSize << endl;

    messageLength = sizeBytes(messageSize + 3);

    outputPacket[1] = messageLength[0];
    outputPacket[2] = messageLength[1];


    // Set the fourth and fifth command bytes
    QByteArray commandCode = commandEncode(command);
    outputPacket[3] = commandCode[0];
    outputPacket[4] = commandCode[1];





    QByteArray encodedMessage;

    if (command == "fapiGetState") {

    } else if (command == "fapiSetDeviceName") {
        encodedMessage = fapiSetDeviceName(message);
    } else if (command == "fapiSetPowerState") {
        encodedMessage = fapiSetPowerState(message);
    } else if (command == "fapiSource") {

    } else if (command == "fapiSetSource") {

    } else if (command == "fapiSetMedia") {

    } else {
        qDebug() << "NO VALID COMMAND IDENTIFIED" << endl;
    }

    outputPacket.insert(5, encodedMessage);




    // Create checksum byte, to append on end of outputPacket

    QByteArray checkSum;

    encodedMessage.prepend(commandCode);


    checkSum = checkSumXor(encodedMessage);

    outputPacket.append(checkSum);

    qDebug() << "YOUR CHOSEN ACTION CALLED THE COMMAND: " << command << "PRODUCING THE FOLLOWING TCP PACKET TO SEND TO THE FUSBUS:" << endl;

    printTcpPacket(outputPacket);


}




/**
 * @brief function to find and return the size of the message itself, in the desired format
 * @param size takes in an int size of the message
 * @return two byte array, of the size in hex form
 */
QByteArray FusionMusicController::sizeBytes(int size)
{
    QByteArray result(2, 0);

    QString hexNum = QString().number(size, 16);


    if (hexNum.size() == 1) {
        hexNum.prepend("000");
    } else if (hexNum.size() == 2) {
        hexNum.prepend("00");
    } else if (hexNum.size() == 3) {
        hexNum.prepend("0");
    } else {

    }

    QString temp = (QString)hexNum[0];
    temp.append(hexNum[1]);

    hexNum[0] = hexNum[2];
    hexNum[1] = hexNum[3];
    hexNum[2] = temp[0];
    hexNum[3] = temp[1];

    result = QByteArray::fromHex(hexNum.toLatin1());

    return result;
}



/**
 * @brief Function that returns hex QByteArray encoding of deviceName
 * @param deviceName
 * @return hex QByteArray
 */
QByteArray FusionMusicController::fapiSetDeviceName(QByteArray &deviceName)
{


    QByteArray example(2, 0);

    example[0] = 0x69;
    example.append("i");

    // messageLength is nameLength + 1, to account for leading length byte; trailing null terminator is automatically accounted for by QByteArray
    int messageLength = deviceName.size() + 1;

    QByteArray message(messageLength, 0);

    message[0] = messageLength;

    for (int i = 0; i < deviceName.size(); i++)
    {
        message[i + 1] = deviceName[i];
    }



    return message;
}



/**
 * @brief function that can be used to map 'ON'/'OFF' state of webpage UI, with corresponding on/off flag for fapiSetPowerState command
 * @param state
 * @return QByte Array of length 1, either 0x01, or 0x02
 */
QByteArray FusionMusicController::fapiSetPowerState(QByteArray &state)
{
    // Note ternary operator could've been used here instead, but verboseness is to help with debugging
    QByteArray powerState(1, 0);

    if (state == "ON")
    {
        powerState[0] = 0x01;

    } else if (state == "OFF")
    {
        powerState[0] = 0x02;
    } else {
       qDebug() << "ERROR! Request is neither ON nor OFF." << endl;
    }



    return powerState;
}




/**
 * @brief function that runs through input QByteArray, reads each array element, and calculates a running XOR checksum, to append to the end of the over-arching TCP packet
 * @param input
 * @return QByteArray of length one, containing XOR checksum byte.
 */
QByteArray FusionMusicController::checkSumXor(QByteArray &input)
{
    int XOR = 0;
    int i;

    QByteArray result;

    for (i = 0; i <= input.size(); i++)
    {
        XOR ^= input[i];

    }


    result[0] = XOR;

    return result;

}



void FusionMusicController::printTcpPacket(QByteArray &input)
{
    QByteArray hexInput = input.toHex();

    QByteArray output;

    int count = 0;


    for (int i = 0; i < hexInput.size(); i+=2)
    {
        qDebug() << hexInput[i] << hexInput[i + 1] << endl;

    }

}










void FusionMusicController::returnJson(HttpRequest &request, HttpResponse &response)
{

}


void FusionMusicController::returnJs(HttpRequest &request, HttpResponse &response)
{

//    response.write("", true);

    response.write("function changeName() { document.getElementById('setDeviceName').value = document.getElementById('name').innerHTML; document.getElementById('name').style.visibility = 'hidden'; document.getElementById('setDeviceName').style.visibility = 'visible'; document.getElementById('changeButton').innerHTML = 'DONE'; document.getElementById('changeButton').onclick = doneName; document.getElementById('setDeviceName').focus(); document.getElementById('setDeviceName').select(); } function doneName() { var name = document.getElementById('setDeviceName').value; if (name.length > 10) { document.getElementById('warningLabel').innerHTML = 'Warning! Name cannot be longer than 10 characters.'; document.getElementById('warningLabel').style.visibility = 'visible'; } else if (name.length == 0) { document.getElementById('warningLabel').innerHTML = 'Warning! Name must be at least one character long.'; document.getElementById('warningLabel').style.visibility = 'visible'; } else { document.getElementById('warningLabel').style.visibility = 'hidden'; document.getElementById('name').innerHTML = name; document.getElementById('setDeviceName').value = ''; document.getElementById('name').style.visibility = 'visible'; document.getElementById('setDeviceName').style.visibility = 'hidden'; document.getElementById('changeButton').innerHTML = 'CHANGE'; document.getElementById('changeButton').onclick = changeName; sendFormData('deviceName', name); } } function playPause() { var songState; if (document.getElementById('playPauseButton').innerHTML == 'Play') { songState = 'PAUSED'; document.getElementById('playPauseButton').innerHTML = 'Pause'; } else { songState = 'PLAYING...'; document.getElementById('playPauseButton').innerHTML = 'Play'; } sendFormData('songState', songState); } function powerChange() { var powerState; if (document.getElementById('switch').checked) { document.getElementById('onLabel').innerHTML = 'ON'; document.getElementById('onLabel').style.color = '#03c46a'; powerState = 'ON'; } else { document.getElementById('onLabel').innerHTML = 'OFF'; document.getElementById('onLabel').style.color = '#ba3728'; powerState = 'OFF'; } sendFormData('powerState', powerState); } function sourceChange() { var sourceType = document.getElementById('choiceBox').value; sendFormData('sourceType', sourceType); } function sendFormData(attribute, information) { var formData = new FormData(); formData.append(attribute, information); var request = new XMLHttpRequest(); request.open('POST', '/fusionMusic'); request.send(formData); }", true);
}

void FusionMusicController::returnCSS(HttpRequest &request, HttpResponse &response)
{
    response.write("body { background-color: #7a7a7a; font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif; } button, .comboBox { background-color: #03c46a; border: none; color: white; padding: 10px 25px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; position:absolute; top:0; right:0; } .centerElements { top: 0%; position: absolute !important; left: 0 !important; right: 0 !important; margin-left: auto !important; margin-right: auto !important; } .dName { color: #03c46a; text-align: center; font-size: 3em; width: 350px; } #moreInfoButton { width: 200px; height: 30px; color: #ffffff; font-size: 1em; text-align: center; } #setDeviceName { position: absolute; width: 250px; height: 40px; font-size: 1em; top: 13%; visibility: hidden; } #name { top: 7%; color: #ffffff; } #changeButton { top: 22%; color: #ffffff; text-align: center; width: 120px; height: 35px; } #doneButton { top: 22%; color: #ffffff; text-align: center; width: 120px; height: 35px; visibility: hidden; background-color: #03c46a; border: none; padding: 10px 25px; text-decoration: none; display: inline-block; font-size: 16px; } #warningLabel { top: 27%; color: #ad3c2e; text-align: center; width: 200px; height: 30px; font-size: 0.5em; visibility: hidden; } #playPauseButton { top: 35%; color: #ffffff; text-align: center; width: 150px; height: 80px; } #sourceTitle { top: 45%; } #choiceBox { top: 60%; width: 100px; height: 50px; text-align: center; select:active, select:hover { outline: none } select:active, select:hover { outline-color: #03c46a; } } #powerSwitch { top: 70%; } #onLabel { top: 70%; color: #ba3728; text-align: center; font-size: 3em; width: 350px; } .switch { position: relative; display: inline-block; width: 60px; height: 34px; } .switch input {display:none;} .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s; } .slider:before { position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s; } input:checked + .slider { background-color: #03c46a; } input:focus + .slider { box-shadow: 0 0 1px #03c46a; } input:checked + .slider:before { -webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px); } .slider.round { border-radius: 34px; } .slider.round:before { border-radius: 50%; } #foot1 { position: absolute; font-size: 1em; color: #000000; top: 88%; } #foot2 { position: absolute; top: 90%; font-size: 2em; }", true);
}

