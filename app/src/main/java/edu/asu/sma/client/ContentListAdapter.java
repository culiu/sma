package edu.asu.sma.client;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;

import java.util.ArrayList;
import java.util.List;


public class ContentListAdapter extends BaseAdapter {
  private Activity activity;
  private LayoutInflater inflater;
  private List<Bitmap> content = new ArrayList<Bitmap>();

  public ContentListAdapter(Activity activity) {
    this.activity = activity;
  }

  public ContentListAdapter(Activity activity, List<byte[]> contentData) {
    this.activity = activity;
    for (byte[] data : contentData)
      add(data);
  }

  public void add(byte[] data) {
    content.add(BitmapFactory.decodeByteArray(data, 0, data.length));
  }

  public void clear() {
    content.clear();
  }

  @Override
  public int getCount() {
    return content.size();
  }

  @Override
  public Object getItem(int i) {
    return content.get(i);
  }

  @Override
  public long getItemId(int i) {
    return i;
  }

  @Override
  public View getView(int i, View view, ViewGroup viewGroup) {
    if (inflater == null)
      inflater = (LayoutInflater) activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    if (view == null)
      view = inflater.inflate(R.layout.content_list_item, null);
    ImageView iv = (ImageView) view.findViewById(R.id.imageView);
    iv.setScaleType(ScaleType.CENTER_CROP);
    iv.setImageBitmap(content.get(i));

    return view;
  }
}
