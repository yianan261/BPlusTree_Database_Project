import os
import requests
from dotenv import load_dotenv

load_dotenv()
API_KEY = os.getenv("GOOGLE_API_KEY")


class Maps:

    def __init__(self):
        if not API_KEY:
            raise ValueError("Missing GOOGLE_API_KEY in .env file.")

    def get_place_id_from_address(self, address):
        url = "https://places.googleapis.com/v1/places:searchText"
        headers = {
            "Content-Type":
            "application/json",
            "X-Goog-Api-Key":
            API_KEY,
            "X-Goog-FieldMask":
            "places.id,places.displayName,places.formattedAddress"
        }
        payload = {"textQuery": address}

        try:
            response = requests.post(url, headers=headers, json=payload)
            data = response.json()

            if "places" in data and len(data["places"]) > 0:
                place_resource_name = data["places"][0]["id"]
                place_id = place_resource_name.split("/")[-1]
                print(f"Found place_id for '{address}': {place_id}")
                return place_id
            else:
                print(f"No place found for address: {address}")
                return None
        except Exception as e:
            print(f"Exception in get_place_id_from_address: {e}")
            return None

    def get_place_details_from_place_id(self, place_id):
        # new Places API (v1) for basic info
        url = f"https://places.googleapis.com/v1/places/{place_id}"
        headers = {
            "Content-Type": "application/json",
            "X-Goog-Api-Key": API_KEY,
            "X-Goog-FieldMask": "id,displayName,formattedAddress,location"
        }

        try:
            response = requests.get(url, headers=headers)
            data = response.json()

            if "id" in data:
                place_info = {
                    "placeId": data.get("id", ""),
                    "name": data.get("displayName", {}).get("text", ""),
                    "address": data.get("formattedAddress", ""),
                    "latitude":
                    str(data.get("location", {}).get("latitude", "")),
                    "longitude":
                    str(data.get("location", {}).get("longitude", "")),
                    "description": self.fetch_editorial_summary_legacy(
                        place_id)  # legacy API for place description
                }
                print(f"Retrieved details for place_id: {place_id}")
                return place_info
            else:
                print(f"No details found for place_id: {place_id}")
                return None
        except Exception as e:
            print(f"Exception in get_place_details_from_place_id: {e}")
            return None

    def fetch_editorial_summary_legacy(self, place_id):
        """ Fetch editorial summary using the old (legacy) Places Details API """
        url = "https://maps.googleapis.com/maps/api/place/details/json"
        params = {
            "place_id": place_id,
            "fields": "name,editorial_summary",
            "key": API_KEY
        }

        try:
            response = requests.get(url, params=params)
            if response.status_code != 200:
                print(
                    f"Legacy API error fetching editorial_summary for {place_id}"
                )
                return "No description available."

            data = response.json()
            result = data.get("result", {})

            summary = result.get("editorial_summary", {}).get("overview", "")
            if summary:
                return summary
            else:
                return "No description available."
        except Exception as e:
            print(f"Exception in fetch_editorial_summary_legacy: {e}")
            return "No description available."
