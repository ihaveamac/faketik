# faketik

faketik generates and installs "fake tickets" to make missing titles re-appear on Nintendo 3DS family systems.

License: MIT.

# Frequently Asked Questions

## My game didn't appear / it found no titles to fix!
The most likely reason for this is that the system does not know that the title exists.

If a Nintendo DSiWare title did not appear, this is likely due to an old CTRNAND image being restored, which means an older `/dbs/title.db` file was restored that does not contain the DSiWare title's information. The easiest method to fix this is to back up saves from TWLNAND, delete, and re-install the titles.

## The message "The External Title Database is not available." appeared!
This happens because `title.db` does not exist where the Nintendo 3DS is attempting to load it (`/Nintendo 3DS/{id0}/{id1}/dbs/title.db`). The database must be restored or re-built. Tools to perform this easily don't exist yet, so it has to be done somewhat manually.

## It's installing a bunch of 0000000000000000 tickets / the message "Failed to allocate memory for the ticket buf." appeared!
This means the external title database does not exist. This issue was fixed in v1.1.

## My game doesn't work!
This is not due to faketik. This only creates tickets to make titles re-appear. It does not fix broken titles or missing seeds.

There is a potential case for DLC titles with over 256 contents, since the current ticket only covers 256 bits in the Content Index. The best way to fix this is to delete the ticket and re-install a legitimate one from Nintendo eShop.
