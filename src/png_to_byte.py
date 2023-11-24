import os
import string
import subprocess

import cv2

SCRIPT_PATH = os.path.dirname(os.path.abspath(__file__))
TILES_PATH = os.path.join(os.path.dirname(SCRIPT_PATH), "assets")
ASSETS_ALL = os.path.join(SCRIPT_PATH, "assets_all.inc")
ASSETS_STR = (
    "{0}\n"
    "\n"
    "const std::unordered_map<Element, std::vector<unsigned char>> img_asset_map {{\n"
    "{1}"
    "}};\n"
)


def main():
    # asset_headers = ""
    asset_mappings = ""
    assets_data = ""
    for file in os.listdir(TILES_PATH):
        if file.endswith(".png"):
            img = cv2.cvtColor(
                cv2.imread("{}/{}".format(TILES_PATH, file)), cv2.COLOR_BGR2RGB
            ).flatten()
            asset_name = file[:-4]
            binary_file_name = asset_name + ".bin"
            binary_file_path = TILES_PATH + "/" + binary_file_name
            img = bytearray(img)
            with open(binary_file_path, "wb") as f:
                f.write(img)

            # create header
            session = subprocess.run(
                ["xxd", "-i", binary_file_name],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=TILES_PATH,
            )
            stdout = session.stdout.decode("utf-8")
            stdout = stdout.replace("unsigned", "static const unsigned")
            stdout = stdout.replace(
                "static const unsigned char {}_bin[]".format(asset_name),
                "static const std::vector<unsigned char> {}_bin".format(asset_name),
            )

            assets_data += stdout

            os.remove(binary_file_path)

            # Add to master include list
            internal_name = "Element::k" + string.capwords(
                asset_name.replace("_", " ")
            ).replace(" ", "")
            asset_mappings += "{{ {0}, {1}_bin }},\n".format(internal_name, asset_name)

    # create master include
    print(ASSETS_STR.format(assets_data, asset_mappings))
    with open(ASSETS_ALL, "w") as f:
        f.write(ASSETS_STR.format(assets_data, asset_mappings))


if __name__ == "__main__":
    main()
