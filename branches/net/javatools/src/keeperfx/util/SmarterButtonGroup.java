package keeperfx.util;

import javax.swing.AbstractButton;
import javax.swing.ButtonGroup;
import javax.swing.ButtonModel;

public class SmarterButtonGroup extends ButtonGroup {
	private static final long serialVersionUID = 35317052798308808L;

	public void selectOne(String action) {
		for (AbstractButton button : buttons) {
			setSelected(button.getModel(), button.getActionCommand().equals(action));
		}
	}
	
	public void setSelection(String action, boolean state) {
		for (AbstractButton button : buttons) {
			if (button.getModel().getActionCommand().equals(action)) {
				button.setSelected(state);
			}
		}
	}
	
	public String getSelected() {
		ButtonModel selected = getSelection();
		if (selected == null) {
			return null;
		}
		
		return selected.getActionCommand();
	}
}
