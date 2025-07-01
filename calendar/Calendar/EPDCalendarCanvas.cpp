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
