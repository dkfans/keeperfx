package keeperfx.configtool;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public abstract class ConfigurationBuffer {
	protected final StringBuffer data = new StringBuffer();
	private final Pattern pattern;
	private final String encoding;
	
	public ConfigurationBuffer(String pattern, String encoding) {
		this.pattern = Pattern.compile(pattern);
		this.encoding = encoding;
	}
	
	public void replace(String string) {
		clear();
		data.append(string);
	}
	
	public void loadFromStream(InputStream is) throws IOException {
		clear();
		
		InputStreamReader reader = new InputStreamReader(is, encoding);
		char[] buf = new char[0x1000];
		
		for (;;) {
			int nRead = reader.read(buf);
			
			if (nRead == -1) {
				break;
			}
			
			data.append(buf, 0, nRead);
		}
		
		if (data.charAt(data.length() - 1) == 26) {
			data.deleteCharAt(data.length() - 1);
		}
	}
	
	@Override
	public String toString() {
		return data.toString();
	}
	
	public void saveToStream(OutputStream os) throws IOException {
		OutputStreamWriter writer = new OutputStreamWriter(os, encoding);
		writer.write(data.toString());
		writer.flush();
	}
	
	public void clearItem(String key) {
		Matcher matcher = pattern.matcher(data);
		
		while (matcher.find()) {
			if (matcher.group(1).trim().equals(key)) {
				data.delete(matcher.start(1), Math.max(matcher.end(1), matcher.end(2)));
				break;
			}
		}
	}
	
	public void setItem(String key, String value) {
		Matcher matcher = pattern.matcher(data);
		boolean found = false;
		
		while (matcher.find()) {
			if (matcher.group(1).trim().equals(key)) {
				found = true;
				
				if (value != null) {
					if (matcher.end(2) < 0) {
						//key without value when we expect value - delete it and start from scratch
						data.delete(matcher.start(1), Math.max(matcher.end(1), matcher.end(1)));
						found = false;
					}
					else {
						data.replace(matcher.start(2), matcher.end(2), value);
					}
				}
				break;
			}
		}
		
		if (!found) {
			newItem(key, value);
		}
	}
	
	protected abstract void newItem(String key, String value);
	
	public boolean hasItem(String key) {
		Matcher matcher = pattern.matcher(data);
		
		while (matcher.find()) {
			if (matcher.group(1).trim().equals(key)) {
				return true;
			}
		}
		
		return false;
	}
	
	public String getItem(String key) {
		Matcher matcher = pattern.matcher(data);
		
		while (matcher.find()) {
			if (matcher.group(1).trim().equals(key)) {
				String value = matcher.group(2);
				if (value != null) {
					value = value.trim();
				}
				
				return value;
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

	private void clear() {
		data.delete(0, data.length());
	}
}
