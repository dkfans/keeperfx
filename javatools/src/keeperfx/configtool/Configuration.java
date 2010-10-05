package keeperfx.configtool;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Configuration {
	private final StringBuffer data = new StringBuffer();
	private final Pattern pattern = Pattern.compile("(.+)=(.+)");
	
	public void loadFromStream(InputStream is) throws IOException {
		clear();
		
		InputStreamReader reader = new InputStreamReader(is, "ISO-8859-1");
		char[] buf = new char[0x1000];
		
		for (;;) {
			int nRead = reader.read(buf);
			
			if (nRead == -1) {
				break;
			}
			
			data.append(buf, 0, nRead);
		}
	}
	
	public void saveToStream(OutputStream os) throws IOException {
		OutputStreamWriter writer = new OutputStreamWriter(os, "ISO-8859-1");
		writer.write(data.toString());
	}
	
	public void setItem(String key, String value) {
		Matcher matcher = pattern.matcher(data);
		boolean found = false;
		
		while (matcher.find()) {
			if (matcher.group(1).trim().equals("key")) {
				found = true;
				
				data.replace(matcher.start(2), matcher.end(2), value);
			}
		}
		
		if (!found) {
			data.append(key);
			data.append('=');
			data.append(value);
			data.append('\n');
		}
	}
	
	public String getItem(String key) {
		Matcher matcher = pattern.matcher(data);
		
		while (matcher.find()) {
			if (matcher.group(1).trim().equals(key)) {
				return matcher.group(2).trim();
			}
		}
		
		return null;
	}
	
	public String getItem(String key, String defaultValue) {
		String value = getItem(key);
		if (value == null) {
			return defaultValue;
		}
		
		return value;
	}

	public void replace(String config) {
		clear();
		data.append(config);
	}

	private void clear() {
		data.delete(0, data.length());
	}
}
