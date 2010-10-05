package keeperfx.configtool.items;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JCheckBox;

import keeperfx.configtool.Configuration;
import keeperfx.util.ValueObservable;

public class CensorshipItem extends ConfigurationItem implements ActionListener {
	private static final long serialVersionUID = -7551853694334766596L;
	private final JCheckBox checkbox;

	public CensorshipItem(ValueObservable<Boolean> configChanged) {
		super(configChanged, "Censorship:");
		
		checkbox = new JCheckBox();
		checkbox.addActionListener(this);
		add(checkbox);
	}

	@Override
	public void load(Configuration config) {
		checkbox.setSelected(config.getItem("CENSORSHIP", "OFF").equals("ON"));
	}

	@Override
	public void save(Configuration config) {
		config.setItem("CENSORSHIP", checkbox.isSelected()? "ON" : "OFF");
	}

	@Override
	public void actionPerformed(ActionEvent ev) {
		markChange();
	}
}
