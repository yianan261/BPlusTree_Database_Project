import os
import csv
import argparse
from maps import Maps


def process_file(input_path, output_folder, maps):
    results = []

    with open(input_path, 'r', encoding='utf-8') as f:
        reader = csv.reader(f)
        headers = next(reader)

        for row in reader:
            if not row:
                continue

            url_or_address = row[0].strip()
            print(f"Processing: {url_or_address}")

            place_id = maps.get_place_id_from_address(url_or_address)
            if place_id:
                details = maps.get_place_details_from_place_id(place_id)
                if details:
                    results.append(details)

    base_filename = os.path.splitext(os.path.basename(input_path))[0]
    output_csv_path = os.path.join(output_folder,
                                   f"{base_filename}_enriched.csv")
    with open(output_csv_path, 'w', encoding='utf-8', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(
            ["placeId", "name", "address", "latitude", "longitude"])
        for place in results:
            writer.writerow([
                place["placeId"], place["name"], place["address"],
                place["latitude"], place["longitude"]
            ])

    print(
        f"Finished {input_path}. Saved {len(results)} places to {output_csv_path}"
    )


def main():
    parser = argparse.ArgumentParser(
        description=
        "Fetch place details and update all CSVs in a folder with place info")
    parser.add_argument("input_folder", help="Folder of saved places CSVs")
    parser.add_argument("user_email", help="User email for folder name")
    args = parser.parse_args()

    input_folder = args.input_folder
    user_email = args.user_email

    safe_user_email = user_email.replace("@", "_at_").replace(".", "_dot_")
    output_folder = f"./saved_places_dir/{safe_user_email}"
    os.makedirs(output_folder, exist_ok=True)

    maps = Maps()

    for filename in os.listdir(input_folder):
        if filename.endswith(".csv"):
            input_path = os.path.join(input_folder, filename)
            print(f"\n Processing file: {input_path}")
            process_file(input_path, output_folder, maps)


if __name__ == "__main__":
    main()
