// Copyright (c) F4HWN Armel. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pgmspace.h>  // PROGMEM support header

const char index_m5stack_html[] = R"HTML(

<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title>ICMultiMeter Screen Capture</title>
    <style>
      html, body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
        width: 100%;
        height: 100%;
        display: table;
        text-align: center;
        background-color: #313131;
      }
      #content {
        display: table-cell;
        text-align: center;
        vertical-align: middle;
      }
      table {
        width: 396px;
        height: 396px;
        border-color: #000; 
        margin-left: auto; 
        margin-right: auto;
        background-image: url('data:image/png;base64,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');
      }
      td {
        padding-top:  10px;
      }
      ul li {
        display: inline-block;
        margin-right: 40px;
      }
      .screen {
        position: relative;
        top: 40px; 
        left: 0px;
        width: 320px;
        height: 240px;
      }
      .button {
        position: relative;
        top: 0px; 
        left: 0px;
        width: 58px;
        height: 31px;
        background-image: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADoAAAAfCAMAAACBBC6fAAAAb1BMVEXCv8vIxdLHxNGJh5CGg4zEwc7Hw9C8ucUAAABxb3ViYWasqbO5tsGIho9FREikoauEgYp8eoIwLzIrKiyoprB1dHtOTVFMS1AhISIVFRUKCgu/vMi1sr2mpK6Zl6GZl5+PjZaAfoZhX2U+PUE8PD/dAsO5AAAAhklEQVQ4y+3VSRaEIAxF0SASggUKaFlW37n/NdoswHOSMW9+p/8DEWWXYs0ofs/tylY6J4+qYnX/tBudggF2putvQMELKNp3hmunQJDxFxiHSkRtA78TyqiDptBCCy30kI6Dkm6TfBGFO2zsKwNNGvkUn9v60z89UPGyev8cot7FoDnV+9MtFNUK0R3IuTMAAAAASUVORK5CYII=');
      }
    </style>
  </head>
  <body>
    <div id="content">
      <table>
        <tr >
            <td>
              <a href="/">
                <img class="screen" alt="Screen" src="screenshot.bmp"/>
              </a>
            </td>
        </tr>
        <tr>
          <td>
            <ul>
              <li><a href="buttonLeft"><div type="button" class="button"></div></a></li>
              <li><a href="buttonCenter"><div type="button" class="button"></div></a></li>
              <li><a href="buttonRight"><div type="button" class="button"></div></a></li>
            </ul>
          </td>
        </tr>
      </table>
    </div>
  </body>
</html>

)HTML";

const char index_core2_html[]  = R"HTML(

<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title>ICMultiMeter Screen Capture</title>
    <style>
      html, body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
        width: 100%;
        height: 100%;
        display: table;
        text-align: center;
        background-color: #313131;
      }
      #content {
        display: table-cell;
        text-align: center;
        vertical-align: middle;
      }
      table {
        width: 396px;
        height: 396px;
        border-color: #000; 
        margin-left: auto; 
        margin-right: auto;
        background-image: url('data:image/png;base64,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');
      }
      td {
        padding-top:  10px;
      }
      ul li {
        display: inline-block;
        margin-right: 40px;
      }
      .screen {
        position: relative;
        top: 40px; 
        left: 0px;
        width: 320px;
        height: 240px;
      }
      .button {
        position: relative;
        top: -12px; 
        left: 0px;
        width: 45px;
        height: 31px;
      }
    </style>
  </head>
  <body>
    <div id="content">
      <table>
        <tr >
            <td>
              <a href="/">
                <img class="screen" alt="Screen" src="screenshot.bmp"/>
              </a>
            </td>
        </tr>
        <tr>
          <td>
            <ul>
              <li><a href="buttonLeft"><div type="button" class="button"></div></a></li>
              <li><a href="buttonCenter"><div type="button" class="button"></div></a></li>
              <li><a href="buttonRight"><div type="button" class="button"></div></a></li>
            </ul>
          </td>
        </tr>
      </table>
    </div>
  </body>
</html>

)HTML";