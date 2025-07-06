#include "EPDCalendarCanvas.h"

void EPDCalendarCanvas::printDayOfWeek(int wday) {
  switch (wday) {
    case 0: print("Zondag"); break;
    case 1: print("Maandag"); break;
    case 2: print("Dinsdag"); break;
    case 3: print("Woensdag"); break;
    case 4: print("Donderdag"); break;
    case 5: print("Vrijdag"); break;
    case 6: print("Zaterdag"); break;
  }
}

void EPDCalendarCanvas::printMonth(int mon) {
  switch (mon) {
    case 0: print("Januari"); break;
    case 1: print("Februari"); break;
    case 2: print("Maart"); break;
    case 3: print("April"); break;
    case 4: print("Mei"); break;
    case 5: print("Juni"); break;
    case 6: print("Juli"); break;
    case 7: print("Augustus"); break;
    case 8: print("September"); break;
    case 9: print("Oktober"); break;
    case 10: print("November"); break;
    case 11: print("December"); break;
  }
}

void EPDCalendarCanvas::displayDateInfo(struct tm * timeinfo) {
  setFont(&FreeSans30pt7b);
  drawText(4,55,String(timeinfo->tm_mday));
  setFont(&FreeSans16pt7b);
  setCursor(80,38);
  printMonth(timeinfo->tm_mon);
  setFont(&FreeSans18pt7b);
  setCursor(4,100);
  printDayOfWeek(timeinfo->tm_wday);
}

int EPDCalendarCanvas::getDayColumn(int wday, int mday, int day) {
  int corwday = (wday==0 ? 6 : (wday-1));
  return (35+day-mday+corwday) % 7;
}

int EPDCalendarCanvas::getDayRow(int wday, int mday, int day) {
  return (day+5-getDayColumn(wday,mday,day)) / 7;
}

int EPDCalendarCanvas::getLastDayOfMonth(int mon, int year) {
  switch (mon) {
    case 0: return 31; break;
    case 1: return ((((year % 400)==0) || (((year % 4)==0) && ((year % 100)!=0))) ? 29: 28); break;
    case 2: return 31; break;
    case 3: return 30; break;
    case 4: return 31; break;
    case 5: return 30; break;
    case 6: return 31; break;
    case 7: return 31; break;
    case 8: return 30; break;
    case 9: return 31; break;
    case 10: return 30; break;
    case 11: return 31; break;
    default: return 0; break;
  }
}

void EPDCalendarCanvas::displayMonthInfo(struct tm * timeinfo) {
  setFont(&FreeSansBold8pt7b);
  drawText(20,150,"MA",1);
  drawText(50,150,"DI",1);
  drawText(80,150,"WO",1);
  drawText(110,150,"DO",1);
  drawText(140,150,"VR",1);
  drawText(170,150,"ZA",1);
  drawText(200,150,"ZO",1);
  drawLine(5,154,215,154,0);

  for (int day=1; day<=getLastDayOfMonth(timeinfo->tm_mon, 1900+timeinfo->tm_year); day++) {
    if ((timeinfo->tm_mday-day)!=0) { //Don't draw the current day, that day will be in RED
      drawText(20+30*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,day),170+20*getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,day),String(day),1);
    }
  }
}

void EPDCalendarCanvas::displayMonthInfoCurrentDay(struct tm * timeinfo) {
  int16_t xpos = 20+30*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday);
  int16_t ypos = 170+20*getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday);
  fillCircle(xpos,ypos-5,10,1); //Red filling
  setTextColor(0, 1); //Red background, white text
  setFont(&FreeSansBold8pt7b);
  drawText(xpos,ypos,String(timeinfo->tm_mday),1);
}

void EPDCalendarCanvas::displayMinMaxTemperature(int minTemp, int maxTemp) {
  setFont(&FreeSans10pt7b);
  drawText(5,310,"MIN");
  setFont(&FreeSans18pt7b);
  drawText(55,320,String(minTemp));
  setFont(&FreeSansBold8pt7b);
  drawText(getCursorX(),305,"o"); //Degrees symbol
  setFont(&FreeSans10pt7b);
  drawText(5,350,"MAX");
  setFont(&FreeSans18pt7b);
  drawText(55,360,String(maxTemp));
  setFont(&FreeSansBold8pt7b);
  drawText(getCursorX(),345,"o"); //Degrees symbol
}

void EPDCalendarCanvas::displayForecast(const String &forecase) {
  setFont(&FreeSans10pt7b);
  drawTextRect(10,420,220,20,forecase);
}

void EPDCalendarCanvas::displayWeatherIconRain(char image, int rainperc) {
  setFont(&WeatherIcons50pt7b);
  setCursor(130,355);
  print(image);

  setFont(&FreeSansBold8pt7b);
  drawText(178,370,String(rainperc)+"%",1);
}

void EPDCalendarCanvas::displayHourWeather(int index, char image, int temp, int winddeg, int windbft, double rain) {
  setFont(&WeatherIcons36pt7b);
  setCursor(10+160*index,100);
  print(image);
  //Temperature
  setFont(&FreeSans16pt7b);
  drawText(80+160*index,65,String(temp));
  setFont(&FreeSansBold8pt7b);
  drawText(getCursorX(),52,"o"); //Degrees symbol
  //Wind direction icon
  drawCircle(100+160*index,95,10,0);
  fillRotatedTriangle(100+160*index, 95, 8, winddeg-90); //North = 0, but geometrically 0 is on the Y-axis!
  //Rain, wind
  setFont(&FreeSansBold8pt7b);
  drawText(45+160*index,128,String(rain,1)+" mm",1);
  drawText(100+160*index,128,String(windbft)+" Bft",1);
}

void EPDCalendarCanvas::displayMonthCalendar(struct tm * timeinfo) {

  //Fixed background, draw rectangles
  int dayStart = 12;
  for (int j = 0; j<2; j++) {
    for (int i = 0; i <7; i++) {
      drawRect(1+113*i, 139+170*j, 113, 170, 0);
    }
  }

  //Print numbers
  setFont(&FreeSans16pt7b);
  int currentDayRow = getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday);
  int lastDayOfMonth = getLastDayOfMonth(timeinfo->tm_mon, 1900+timeinfo->tm_year);
  for (int day=1; day<=(lastDayOfMonth+13); day++) { //Should continu for max 13 days more!
    int row = getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,day);
    int daynr = (day<=lastDayOfMonth) ? day : day-lastDayOfMonth;
    if ((row>=currentDayRow) && (row<currentDayRow+2)) { //Only draw the row with the current day and the next one
      if ((timeinfo->tm_mday-day)!=0) { //Don't draw the current day, that day will be in RED
        drawText(57+113*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,day),168+170*(row-currentDayRow),String(daynr),1);
      }
    }
  }
}

void EPDCalendarCanvas::displayMonthCalendarCurrentDay(struct tm * timeinfo) {
  setTextColor(1, 0); //White background, red text
  setFont(&FreeSans16pt7b);
  drawText(57+113*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday),168,String(timeinfo->tm_mday),1);
}

void EPDCalendarCanvas::displayMonthCalendarEntry(struct tm * timeinfo, int index, int line, const String description) {
  setFont(&FreeSansBold8pt7b);
  int currentDayRow = getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday);
  int row = getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday+index);
  if ((row>=currentDayRow) && (row<currentDayRow+2)) {
    drawText(6+113*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,index+timeinfo->tm_mday),188+170*(row-currentDayRow)+18*line,description);
  }
}

void EPDCalendarCanvas::displayCalendarWeather(struct tm * timeinfo, int index, char image, int mintemp, int maxtemp, int rainperc) {

  int currentDayRow = getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday);
  int row = getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday+index);
  int16_t xpos = 1+113*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday+index);
  int16_t ypos = 300+170*(row-currentDayRow);

  setFont(&WeatherIcons26pt7b);
  setCursor(xpos,ypos+5);
  print(image);

  setFont(&FreeSansBold8pt7b);
  drawText(49+xpos,ypos-22,String(mintemp)+" > "+String(maxtemp));
  drawText(49+xpos,ypos-2,String(rainperc)+"%");
}

boolean EPDCalendarCanvas::calendarSpaceAvailable() {
  return (getCursorY()<430);
}

void EPDCalendarCanvas::displayCalendarResetDayCursor() {
  dayCursor = 0;
}

void EPDCalendarCanvas::displayCalendarEntryUrgent(int mday, char type, boolean displayType) {
  int16_t ypos = getCursorY(); //16 (-10) standaard
  if (mday!=dayCursor) {
    dayCursor = mday;
    ypos+=10; //Add 10 pixels for separation between days
  }
  if (displayType) {
    fillCircle(390,12+ypos,10,1); //Red
    setTextColor(0, 1); // white text, red background
    setFont(&FreeSansBold8pt7b);
    drawText(390,18+ypos,String(type),1);
  }
  setCursor(308,ypos+50);
}

void EPDCalendarCanvas::displayCalendarEntry(int mday, int wday, int hs, int ms, int he, int me, char type, boolean fullDay, boolean displayType, const String description) {
  int16_t ypos = getCursorY(); //16 (-10) standaard
  if (mday!=dayCursor) {
    dayCursor = mday;
    ypos+=10; //Add 10 pixels for separation between days
    setFont(&FreeSans16pt7b);
    drawText(298,24+ypos,String(mday),2);
    setFont(&FreeSans10pt7b);
    drawText(298,44+ypos,DAYSOFWEEK[wday],2);
  }
  fillRect(308,ypos,2,50,0);
  if (displayType) {
    fillCircle(390,12+ypos,10,0);
    setTextColor(1, 0); // white text, black background
    setFont(&FreeSansBold8pt7b);
    drawText(390,18+ypos,String(type),1);
  }
  setTextColor(0, 1); // black text, white background
  setFont(&FreeSans10pt7b);
  drawText(405,20+ypos,description);
  if (!fullDay) {
    setFont(&FreeSans12pt7b);
    String msStr = String(ms);
    if (msStr.length()<2) {msStr = "0"+msStr;}
    String meStr = String(me);
    if (meStr.length()<2) {meStr = "0"+meStr;}
    drawText(370,20+ypos,String(hs)+":"+msStr,2);
    setFont(&FreeSansBold8pt7b);
    drawText(370,38+ypos,String(he)+":"+meStr,2);
    drawTimeRect(380,32+ypos,hs,ms,he,me);
  }
  setCursor(308,ypos+50);
}

void EPDCalendarCanvas::displayStatus(struct tm * timeinfo) {
  String minStr = String(timeinfo->tm_min);
  if (minStr.length()<2) {minStr = "0"+minStr;}
  drawText(780,470,"Last updated: "+String(timeinfo->tm_hour)+minStr,2);
}
