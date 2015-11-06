package edu.asu.sma.client;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentTransaction;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import java.net.InetAddress;

import edu.asu.sma.network.WifiConfigurationNew;
import edu.asu.sma.network.WifiManagerNew;


public class NavActivity extends FragmentActivity implements PostPreviewDialog.OnCompleteListner, OnTabChangeListener {
  private WifiManager mWifiMgr;
  private WifiManagerNew mWifiMgrNew;
  private IntentFilter mFilter;
  private BroadcastReceiver mReceiver;

  private TabHost tab_host;
  private int[] tab_icon_res;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.activity_nav);

//    createAdHocNetwork();

    tab_icon_res = new int[]{
        R.drawable.tab_ic_network
        , R.drawable.tab_ic_content
        , R.drawable.tab_ic_post
    };

    tab_host = (TabHost) findViewById(android.R.id.tabhost);
    tab_host.setOnTabChangedListener(this);
    tab_host.setup();

      for (int i = 0; i < tab_icon_res.length; ++i)
          createTab(i);


//    startService(new Intent(this, TimeService.class));
    startService(new Intent(this, NativeService.class));
  }

  @Override
  protected void onStart() {
    super.onStart();
  }

  @Override
  protected void onStop() {
    super.onStop();
  }

  private void shutdown() {
//    stopService(new Intent(this, TimeService.class));
    stopService(new Intent(this, NativeService.class));
    finish();
  }

  private void createAdHocNetwork() {
    mWifiMgr = (WifiManager) getSystemService(WIFI_SERVICE);
    mWifiMgrNew = new WifiManagerNew(mWifiMgr);

    // Register broadcast receiver to get notified when wifi has been enabled.
    mFilter = new IntentFilter();
    mFilter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);

    mReceiver = new BroadcastReceiver() {
      @Override
      public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        if (WifiManager.WIFI_STATE_CHANGED_ACTION.equals(action)) {
          int state = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                  WifiManager.WIFI_STATE_UNKNOWN);

          if (state == WifiManager.WIFI_STATE_ENABLED) {
            Log.d("navactivity", "wifi state enabled");
            if (mWifiMgrNew.isIbssSupported()) {
              Log.d("navactivity", "ad-hoc mode is supported");
              configureAdhocNetwork();
              Log.d("navactivity", "Successfully configured Adhoc network.");
            } else {
              Log.d("navactivity", "ad-hoc mode is not supported.");
            }
          }
        }
      }
    };
  }

  private void configureAdhocNetwork() {
    try {
            /* We use WifiConfigurationNew which provides a way to access
             * the Ad-hoc mode and static IP configuration options which are
             * not part of the standard API yet */
      WifiConfigurationNew wifiConfig = new WifiConfigurationNew();

            /* Set the SSID and security as normal */
      wifiConfig.SSID = "\"smaadhoc\"";
      wifiConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);

            /* Use reflection until API is official */
      wifiConfig.setIsIBSS(true);
      wifiConfig.setFrequency(2442);

            /* Use reflection to configure static IP addresses */
      wifiConfig.setIpAssignment("STATIC");
      wifiConfig.setIpAddress(InetAddress.getByName("224.0.0.11"), 24);
      wifiConfig.setGateway(InetAddress.getByName("224.0.0.1"));
      wifiConfig.setDNS(InetAddress.getByName("224.0.0.1"));

            /* Add, enable and save network as normal */
      int id = mWifiMgr.addNetwork(wifiConfig);
      if (id < 0) {
        Log.d("navactivity", "Failed to add Ad-hoc network");
      } else {
        mWifiMgr.enableNetwork(id, true);
        mWifiMgr.saveConfiguration();
      }
    } catch (Exception e) {
      Log.d("navactivity", "Wifi configuration failed!");
      e.printStackTrace();
    }
  }


  private View createTabView(final int index) {
    View view = LayoutInflater.from(this).inflate(R.layout.tab_content, null);
    ImageView image_view = (ImageView) view.findViewById(R.id.tab_icon);
    image_view.setImageDrawable(getResources().getDrawable(tab_icon_res[index]));
    return view;
  }

  private void createTab(final int index) {
    TabHost.TabSpec spec = tab_host.newTabSpec(Integer.toString(index));
    spec.setContent(new TabHost.TabContentFactory() {
      public View createTabContent(String tag) {
        return findViewById(R.id.realtabcontent);
//        final TextView tv  =new TextView(NavActivity.this);
//        tv.setText("content for tab with tag " + tag);
//        return tv;
      }
    });

    spec.setIndicator(createTabView(index));

    tab_host.addTab(spec);
  }

  @NonNull
  private Fragment createTabFragment(int index) {
    switch (index) {
      case 0:
        return new NetworkFragment();
      case 1:
        return new ContentFragment();
      case 2:
        return new PostFragment();
    }
    throw new IndexOutOfBoundsException(index + " is not a valid tab index");
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    getMenuInflater().inflate(R.menu.nav, menu);
    return super.onCreateOptionsMenu(menu);
  }

  @Override
  public void onTabChanged(String tab_name) {
    final int tab_index = Integer.parseInt(tab_name);

    FragmentTransaction txn = getSupportFragmentManager().beginTransaction();
    txn.replace(R.id.realtabcontent, createTabFragment(tab_index));
    txn.commit();
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
      case R.id.action_upload_logs:
        StatLogger.upload("192.168.1.1", 9996);
        return true;
      case R.id.action_shutdown:
        shutdown();
        return true;
      default:
        return super.onOptionsItemSelected(item);
    }
  }

  public void onComplete(String msg) {
    Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
  }
}
