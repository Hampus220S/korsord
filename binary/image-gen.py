#
# image-gen.py - generate image using openai Dall E
#
# Written by Hampus Fridholm
#
# https://github.com/openai/openai-python
#

from openai import OpenAI
import requests
from PIL import Image
from io import BytesIO
import argparse
import os
from common import *

# Start an openai client using API key
api_key = api_key_get()

if not api_key:
    print(f"korsord: API key not found")
    sys.exit(1)

client = OpenAI(api_key=api_key)

if __name__ == "__main__":
    # Parse command line arguments
    parser = argparse.ArgumentParser(
        description="Image generator"
    )

    parser.add_argument("prompt",
        type=str,
        help="Prompt for image"
    )

    parser.add_argument('--name',
        type=str, default="temp",
        help="Name of image"
    )

    args = parser.parse_args()


    print(f"Generating image")

    # Call the OpenAI API to generate an image
    response = client.images.generate(
      prompt=args.prompt,
      n=1,           # Number of images to generate
      size="256x256" # Image size, you can adjust as needed
    )

    print(f"Generated image")

    # Get the image URL from the response
    image_url = response.data[0].url

    # Fetch the image from the URL
    image_response = requests.get(image_url)
    img = Image.open(BytesIO(image_response.content))

    image_file = image_file_get(args.name)

    img.save(image_file)
