static inline char* _hid_to_unix_path(char* path)
{
	char bus [5];
	char dev [5];
	char *result = malloc( sizeof(char) * ( 20 + 1 ) );

	sprintf (bus, "%.*s", 4, path);
	sprintf (dev, "%.*s", 4, path + 5);

	sprintf (result, "/dev/bus/usb/%03d/%03d",
		(int)strtol(bus, NULL, 16),
		(int)strtol(dev, NULL, 16));
	return result;
}

