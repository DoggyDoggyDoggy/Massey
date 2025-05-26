import yagmail
import gmail_acc

def send_email(timestamp, device_id, soilmoisture):
    yag = yagmail.SMTP(gmail_acc.gmail_email, gmail_acc.gmail_pas)

    contents = f"""\
    Low soil moisture alert!

     Time: {timestamp}  
     Device ID: {device_id}  
     Soil Moisture: {soilmoisture}

    Please take action to water the soil if necessary.

    — IoT Soil Monitoring System
    """

    yag.send(
        to = "pedan_denys@outlook.com",
        subject = "Low Soil Moisture",
        contents = contents
    )