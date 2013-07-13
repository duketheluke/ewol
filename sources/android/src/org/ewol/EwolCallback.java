/**
 * @author Edouard DUPIN, Kevin BILLONNEAU
 *
 * @copyright 2011, Edouard DUPIN, all right reserved
 *
 * @license BSD v3 (see license file)
 */

package org.ewol;

public interface EwolCallback {

    public void keyboardUpdate(boolean show);
    public void eventNotifier(String[] args);
    public void orientationUpdate(int screenMode);

}
