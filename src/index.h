#ifndef WEBPAGE_H
#define WEBPAGE_H

const char* webpage = R"=====(
    <!doctype html>
    <html>
        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1.0" />
            <title>Flight tracker</title>
            <style>
                html {
                    font-family: Verdana, sans-serif;
                }

                .container {
                    max-width: 600px;
                    margin: 50px auto;
                    text-align: center;
                }

                h1 {
                    color: #000;
                }

                .button {
                    display: inline-block;
                    padding: 10px 20px;
                    margin: 10px;
                    font-size: 16px;
                    border: none;
                    border-radius: 5px;
                    cursor: pointer;
                    transition-duration: 0.4s;
                }

                .button-data {
                    background-color: teal;
                    color: #fff;
                }

                .button:hover {
                    background-color: lightseagreen;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>Flight tracker</h1>

                <div>Download the flight data of the last 60 seconds at 10 Hz.</div>

                <a href="download"
                    ><button class="button button-data">Download Data</button></a
                >
            </div>
        </body>
    </html>
)=====";

#endif
