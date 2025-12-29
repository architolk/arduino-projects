#include "CalEntries.h"

void CalEntries::add(CalEntry* &entry) {
  entries.add(entry);
}

int compare(CalEntry* &a, CalEntry* &b) {
  if (a->year < b->year) return -1;
  if (a->year > b->year) return 1;
  if (a->month < b->month) return -1;
  if (a->month > b->month) return 1;
  if (a->mday < b->mday) return -1;
  if (a->mday > b->mday) return 1;
  if (a->startHour < b->startHour) return -1;
  if (a->startHour > b->startHour) return 1;
  if (a->startMinute < b->startMinute) return -1;
  if (a->startMinute > b->startMinute) return 1;
  return 0;
}

void CalEntries::sort() {
  entries.sort(compare);
}

bool CalEntries::first(CalEntry* &entry) {
  current = 0;
  if (entries.size()>0) {
    entry = entries.get(0);
    return true;
  }
  return false;
}

bool CalEntries::next(CalEntry* &entry) {
  current++;
  if (entries.size()>current) {
    entry = entries.get(current);
    return true;
  }
  return false;
}
