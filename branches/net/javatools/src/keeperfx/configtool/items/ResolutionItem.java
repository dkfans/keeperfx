package keeperfx.configtool.items;

import java.awt.Dimension;
import java.awt.DisplayMode;
import java.awt.GraphicsEnvironment;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Comparator;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;
import java.util.TreeSet;

import javax.swing.JComboBox;
import javax.swing.JOptionPane;

import keeperfx.configtool.Configuration;
import keeperfx.util.ValueObservable;

public class ResolutionItem extends ConfigurationItem
		implements Comparator<DisplayMode>, ActionListener {
	private static final long serialVersionUID = 4260855099314334744L;
	private static final int MIN_N_FIELDS = 3;
	
	private final String key;
	private final int valueIndex;
	private final JComboBox combobox;
	private String oldResolution;

	public ResolutionItem(ValueObservable<Boolean> configChanged,
			String label, String key, int valueIndex) {
		super(configChanged, label);

		this.key = key;
		this.valueIndex = valueIndex;
		
		combobox = new JComboBox();
		combobox.setPreferredSize(new Dimension(400, 20));
		add(combobox);
		
		refillModeList("Other...");
	}

	private String modeString(DisplayMode mode) {
		return mode.getWidth() + "x" + mode.getHeight() + "x" + mode.getBitDepth();
	}

	@Override
	public void load(Configuration config) {
		StringTokenizer tokenizer = new StringTokenizer(config.getItem(key, ""));
		String resolution = null;
		try {
			for (int i = 0; i <= valueIndex; ++i) {
				resolution = tokenizer.nextToken();
			}
		}
		catch (NoSuchElementException e) {
			resolution = "640x480x8";
		}
		
		selectResolution(resolution);
	}
	
	private void refillModeList(String oddMode) {
		combobox.removeActionListener(this);
		
		combobox.removeAllItems();
		
		DisplayMode[] displayModes = GraphicsEnvironment.getLocalGraphicsEnvironment().
		getDefaultScreenDevice().getDisplayModes();
		TreeSet<DisplayMode> mergedModes = new TreeSet<DisplayMode>(this);
		
		for (DisplayMode mode : displayModes) {
			mergedModes.add(mode);
		}
		for (DisplayMode mode : mergedModes) {
			combobox.addItem(modeString(mode));
		}
		
		combobox.addItem(oddMode);
		combobox.setSelectedIndex(combobox.getItemCount() - 1);
		
		combobox.addActionListener(this);
	}

	private void selectResolution(String resolution) {
		combobox.setSelectedItem(null); //clears selection
		combobox.setSelectedItem(resolution);
		int index = combobox.getSelectedIndex();
		if (index == -1) {
			refillModeList(resolution);
			combobox.addItem("Other...");
		}
		
		oldResolution = (String) combobox.getSelectedItem();
	}

	@Override
	public void save(Configuration config) {
		String oldVal = config.getItem(key, "");
		String[] oldFields = oldVal.split("\\s+");
		String[] newFields = new String[Math.max(oldFields.length, MIN_N_FIELDS)];

		for (int i = 0; i < oldFields.length; ++i) {
			newFields[i] = oldFields[i];
		}
		for (int i = oldFields.length; i < newFields.length; ++i) {
			newFields[i] = "640x480x8";
		}
		newFields[valueIndex] = (String) combobox.getSelectedItem();
		
		String newVal = "";
		for (String field : newFields) {
			newVal += field + " ";
		}
		
		config.setItem(key, newVal);
	}

	@Override
	public int compare(DisplayMode a, DisplayMode b) {
		if (a.getWidth() != b.getWidth()) {
			return a.getWidth() - b.getWidth();
		}
		if (a.getHeight() != b.getHeight()) {
			return a.getHeight() - b.getHeight();
		}
		if (a.getBitDepth() != b.getBitDepth()) {
			return a.getBitDepth() - b.getBitDepth();
		}
		
		return 0;
	}

	@Override
	public void actionPerformed(ActionEvent ev) {
		//assume resolution combo box is source
		if (combobox.getSelectedIndex() == combobox.getItemCount() - 1) {
			String input = JOptionPane.showInputDialog("Please enter a width x height x bits per pixel triple.",
					oldResolution);
			if (input == null) {
				combobox.setSelectedItem(oldResolution);
			}
			else {
				selectResolution(input);
				markChange();
			}
		}
		else {
			markChange();
		}
	}
}
