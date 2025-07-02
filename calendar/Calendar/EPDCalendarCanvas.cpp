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
  drawLine(5,155,215,155,0);

  for (int day=1; day<=getLastDayOfMonth(timeinfo->tm_mon, timeinfo->tm_year); day++) {
    if ((timeinfo->tm_mday-day)!=0) { //Don't draw the current day, that day will be in RED
      drawText(20+30*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,day),170+20*getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,day),String(day),1);
    }
  }
}

void EPDCalendarCanvas::displayMonthInfoCurrentDay(struct tm * timeinfo) {
  setFont(&FreeSansBold8pt7b);
  drawText(20+30*getDayColumn(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday),170+20*getDayRow(timeinfo->tm_wday,timeinfo->tm_mday,timeinfo->tm_mday),String(timeinfo->tm_mday),1);
}

void EPDCalendarCanvas::displayMinMaxTemperature(double minTemp, double maxTemp) {
  setFont(&FreeSans10pt7b);
  drawText(5,310,"MIN");
  setFont(&FreeSans18pt7b);
  drawText(55,320,String(minTemp,1));
  setFont(&FreeSansBold8pt7b);
  drawText(getCursorX(),305,"o"); //Degrees symbol
  setFont(&FreeSans10pt7b);
  drawText(5,350,"MAX");
  setFont(&FreeSans18pt7b);
  drawText(55,360,String(maxTemp,1));
  setFont(&FreeSansBold8pt7b);
  drawText(getCursorX(),345,"o"); //Degrees symbol
}

void EPDCalendarCanvas::displayForecast(const String &forecase) {
  setFont(&FreeSans10pt7b);
  drawTextRect(10,420,220,20,forecase);
}

void EPDCalendarCanvas::displayWeatherIconRain(char icon, double rain) {
  setFont(&WeatherIcons50pt7b);
  setCursor(130,355);
  print(icon);

  setFont(&FreeSansBold8pt7b);
  drawText(178,370,String(rain,1)+" mm",1);
}

void EPDCalendarCanvas::displayCalendarEvent(int mday, int wday, int hs, int ms, int he, int me, int type, const String &description) {
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
  fillCircle(390,12+ypos,10,0);
  setTextColor(1, 0); // white text, black background
  setFont(&FreeSansBold8pt7b);
  drawText(390,18+ypos,"J",1);
  setTextColor(0, 1); // black text, white background
  setFont(&FreeSans10pt7b);
  drawText(405,20+ypos,description);
  setFont(&FreeSans12pt7b);
  drawText(370,20+ypos,String(hs)+":"+String(ms),2);
  setFont(&FreeSansBold8pt7b);
  drawText(370,38+ypos,String(he)+":"+String(me),2);
  drawTimeRect(380,32+ypos,hs,ms,he,me);
  setCursor(308,ypos+50);
}
