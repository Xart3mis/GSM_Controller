import requests
import json

API_KEY = "pk.1eb53d351789d96db350712fa4983c80"


def get_loc(MCC, MNC, LAC, CELL_ID, API_KEY=API_KEY):
    url = "https://us1.unwiredlabs.com/v2/process.php"
    data = {
        "token": API_KEY,
        "radio": "gsm",
        "mcc": MCC,
        "mnc": MNC,
        "cells": [
            {
                "radio": "gsm",
                "mcc": 602,
                "mnc": 3,
                "lac": 21342,
                "cid": 8063,
            },
            {
                "radio": "gsm",
                "mcc": 602,
                "mnc": 3,
                "lac": 21342,
                "cid": 8064,
            },
            {
                "radio": "gsm",
                "mcc": 602,
                "mnc": 3,
                "lac": 21342,
                "cid": 8053,
            },
            # {   "radio": "gsm",
            #     "mcc": 602,
            #     "mnc": 1,
            #     "lac": int("8184", 16),
            #     "cid": int("6c17", 16)
            # },
            # {   "radio": "gsm",
            #     "mcc": 602,
            #     "mnc": 2,
            #     "lac": int("5533", 16),
            #     "cid": int("12fe", 16)
            # },
            # {   "radio": "gsm",
            #     "mcc": 602,
            #     "mnc": 2,
            #     "lac": int("5533", 16),
            #     "cid": int("12fd", 16)
            # },
        ],
        "address": 1,
    }

    print(json.dumps(data))
    response = requests.post(url, json=data)
    if response.status_code == 200:
        # lat = response.json()[u"location"][u"lat"]
        # long = response.json()[u"location"][u"lng"]
        print(response.json())
        # d = {"LAT": lat, "LONG": long}
        # print("Located Cell: {}".format(ID))


if __name__ == "__main__":
    get_loc(602, 3, 21342, 8063)
