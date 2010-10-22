package keeperfx.util;

import java.util.Observable;

public class ValueObservable<V> extends Observable {
	private V value;
	private final int equalCheck;
	
	public static final int NO_EQUAL_CHECK = 0;
	public static final int REF_EQUAL_CHECK = 1;
	public static final int DEEP_EQUAL_CHECK = 2;
	
	public ValueObservable() {
		this(null, NO_EQUAL_CHECK);
	}
	
	public ValueObservable(V initVal) {
		this(initVal, NO_EQUAL_CHECK);
	}
	
	public ValueObservable(int equalCheckSemantics) {
		this(null, equalCheckSemantics);
	}
	
	public ValueObservable(V initVal, int equalCheckSemantics) {
		value = initVal;
		equalCheck = equalCheckSemantics;
	}
	
	/**
	 * Changes the value and retrieves the old value.
	 * Observers are notified with the _old_ value (they can get new value
	 * from getValue anyhow).
	 * If this object has an equal check evaluating to true nothing will
	 * happen (observers won't be notified).
	 * @param val The new value.
	 * @return The old value.
	 */
	public V setValue(V val) {
		if (equalCheck == REF_EQUAL_CHECK) {
			if (val == value) {
				return value;
			}
		}
		else if (equalCheck == DEEP_EQUAL_CHECK) {
			if (value == null && val == value) {
				return value;
			}
			
			if (value.equals(val)) {
				return value;
			}
		}
		
		V oldVal = value;
		value = val;
		setChanged();
		notifyObservers(oldVal);
		
		return oldVal;
	}
	
	/**
	 * Gets the current value.
	 * @return
	 */
	public V getValue() {
		return value;
	}
}
