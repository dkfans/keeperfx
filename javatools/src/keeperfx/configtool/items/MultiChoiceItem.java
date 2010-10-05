package keeperfx.configtool.items;

import java.awt.GridLayout;

import javax.swing.JRadioButton;

import keeperfx.configtool.Configuration;
import keeperfx.util.SmarterButtonGroup;
import keeperfx.util.ValueObservable;

public class MultiChoiceItem extends ConfigurationItem {
	private static final long serialVersionUID = -6789909728535376842L;
	private final SmarterButtonGroup group;
	private final String key;
	private final String defaultChoice;

	public MultiChoiceItem(ValueObservable<Boolean> configChanged,
			String label, String key, int columns, String[] choices) {
		super(configChanged, label);
		this.key = key;
		this.defaultChoice = choices[0];

		int rows = (int) Math.ceil(choices.length / (double) columns);
		setLayout(new GridLayout(rows, columns));
		
		group = new SmarterButtonGroup();
		for (String choice : choices) {
			addChoice(choice);
		}
	}

	private void addChoice(String name) {
		JRadioButton button = new JRadioButton(name);
		button.setActionCommand(name);
		add(button);
		group.add(button);
	}

	@Override
	public void load(Configuration config) {
		group.selectOne(config.getItem(key, defaultChoice));
	}

	@Override
	public void save(Configuration config) {
		String selected = group.getSelected();
		config.setItem(key, selected == null? defaultChoice : selected);
	}

}
