import re
from datetime import datetime

WEEKDAYS_FULL = ["Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"]
DAY_ALIASES = {
    "monday":0,"mon":0,
    "tuesday":1,"tue":1,"tues":1,
    "wednesday":2,"wed":2,
    "thursday":3,"thu":3,"thur":3,"thurs":3,
    "friday":4,"fri":4,
    "saturday":5,"sat":5,
    "sunday":6,"sun":6,
}

def _parse_time_12h(s: str):
    s = s.strip().lower().replace(" ", "")
    m = re.match(r"^(\d{1,2})(?::(\d{2}))?(am|pm)$", s)
    if not m:
        return None
    h = int(m.group(1))
    mi = int(m.group(2) or 0)
    ampm = m.group(3)
    if h == 12:
        h = 0
    if ampm == "pm":
        h += 12
    return (h, mi)

def _norm_dashes(t: str):
    return t.replace("—","-").replace("–","-").replace("-","-")

def _find_range(text: str):
    t = _norm_dashes(text)
    day = r"(Mon(?:day)?|Tue(?:s|sday)?|Wed(?:nesday)?|Thu(?:r|rs|rsday)?|Fri(?:day)?|Sat(?:urday)?|Sun(?:day)?)"
    time_pat = r"([0-9]{1,2}(?::[0-9]{2})?\s*(?:am|pm))"
    pat = rf"{day}\s*-\s*{day}\s*,?\s*{time_pat}\s*-\s*{time_pat}"
    m = re.search(pat, t, flags=re.IGNORECASE)
    if not m:
        return None
    d1_raw, d2_raw, s_start, s_end = m.group(1), m.group(2), m.group(3), m.group(4)
    d1 = DAY_ALIASES.get(d1_raw.lower()); d2 = DAY_ALIASES.get(d2_raw.lower())
    start = _parse_time_12h(s_start); end = _parse_time_12h(s_end)
    if d1 is None or d2 is None or not start or not end:
        return None
    if d1 <= d2:
        covered = list(range(d1, d2+1))
    else:
        covered = list(range(d1, 7)) + list(range(0, d2+1))
    return (tuple(covered), start, end)

def is_open_now_from_text(hours_text: str, now: datetime | None = None):
    if now is None:
        now = datetime.now()

    found = _find_range(hours_text)
    if not found:
        return (None, "I couldn't parse the hours from the page.", None)

    covered_days, (sh, sm), (eh, em) = found
    weekday = now.weekday()

    def _fmt(h, m):
        suf = "AM" if h < 12 else "PM"
        h12 = h % 12 or 12
        return f"{h12}:{m:02d} {suf}"

    range_human = f"{_fmt(sh,sm)}–{_fmt(eh,em)}"
    now_minutes = now.hour*60 + now.minute
    start_minutes = sh*60 + sm
    end_minutes = eh*60 + em

    if weekday not in covered_days:
        msg = (f"Today is {WEEKDAYS_FULL[weekday]}; regular hours are {range_human}, "
               f"{WEEKDAYS_FULL[covered_days[0]]}–{WEEKDAYS_FULL[covered_days[-1]]}.")
        return (False, msg, range_human)

    if start_minutes <= now_minutes <= end_minutes:
        left = end_minutes - now_minutes
        hrs, mins = divmod(left, 60)
        left_str = f"{hrs}h {mins}m" if hrs else f"{mins}m"
        return (True, f"Open now; closes in about {left_str}.", range_human)
    else:
        if now_minutes < start_minutes:
            return (False, f"Closed now; opens today at {_fmt(sh,sm)}.", range_human)
        return (False, f"Closed now; closed for the day (regular hours {range_human}).", range_human)
